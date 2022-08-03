#include "download.h"
#include <string.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include "fileutil.h"
#include <iostream>
#include "workerthread.h"
#include <functional>

class Response
{
public:
	Response() : resultCode(0), valid(true), msg(), fname() {}
	int resultCode;
	bool valid;
	std::string msg;
	std::string fname;
};

#ifdef ALLEGRO_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USE_CURL
#include <curl/curl.h>
#endif

using namespace std;

#ifdef USE_CURL
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
int progress_callback(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow);
#endif

#ifdef USE_CURL
// copied from curl example: http://curl.haxx.se/libcurl/c/url2file.html
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

int progress_callback(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow)
{
	// the callback is called about once per second, even when nothing is downloading.
	// we use it to signal thread interruption.
	auto worker = static_cast<WorkerThread*>(clientp);
	int result = worker->isInterrupted() ? 1 : 0;
	return result;
}

#endif

class HttpWorker : public WorkerThread {
protected:
	string userAgent;

public:	
	HttpWorker() : userAgent("libcurl-twist/1.0")
	{
	}

	void setUserAgent(const string &value)
	{
		userAgent = value;
	}

};

class PostWorker : public HttpWorker {
private:
	const string url;
	const string body;
public:
	PostWorker(const string &url, const string &body) : url(url), body(body) {}

	virtual Response post(const string &url, const string &body) {

		Response result = Response();
		result.valid = false;
	#ifdef USE_CURL
		CURL *curl;
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0); // enable progress callback
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this); // passed as first argument to progress function
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback); // progress callback allows us to interrupt
			/* Now specify the POST data */ 
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // in seconds
			curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

			CURLcode res;
			
			/* get it! */
			res = curl_easy_perform(curl);

			result.resultCode = res;
			if(res != CURLE_OK)
			{
				fprintf(stderr, "curl_easy_perform() failed: %s on url %s\n",
						curl_easy_strerror(res), url.c_str());
			}
			curl_easy_cleanup(curl);
		}
		else {
			result.msg = "curl_easy_init() failed";
		}
		curl_global_cleanup();
	#else
		result.msg = "libCurl not linked, compile with -DUSE_CURL or contact developers";
	#endif
		return result;
	}

	void run() override {
		Response response = post(url, body);
	}
};

class DownloadWorker : public HttpWorker {
private:
	const string url;
	const string fname;
public:
	DownloadWorker (const string &url, const string &fname) : url(url), fname(fname) {
	}

	virtual Response downloadToFile(const string &url, const string &fname)
	{
		Response result = Response();
		result.valid = false;
	#ifdef USE_CURL

		string tempnam = fname + ".tmp";
		
		CURL *curl;
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // in seconds
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // data writing callback
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback); // progress callback allows us to interrupt
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this); // reference to this for progress callback
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0); // enable progress callback
		curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

		CURLcode res;
		FILE *pagefile;
		pagefile = fopen(tempnam.c_str(), "wb");
		if(pagefile) {

			/* write the page body to this file handle */
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

			/* get it! */
			res = curl_easy_perform(curl);

			/* close the header file */
			fclose(pagefile);
			
			result.resultCode = res;
			if(res != CURLE_OK)
			{
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));

				result.msg = curl_easy_strerror(res);

				if( remove( tempnam.c_str() ) != 0 )
					perror( "Error deleting temporary file" );
				else
					puts( "Temporary file successfully deleted" );
			}
			else
			{
				result.valid = true;
#ifdef ALLEGRO_WINDOWS
				// on windows, use MoveFileEx if we want to overwrite existing file
				int err = MoveFileEx (tempnam.c_str(), fname.c_str(), MOVEFILE_REPLACE_EXISTING);
				if (err == 0)
				{
					cout << "MoveFileEx failed: " << GetLastError() << endl;
				}
#else
				// on *nix, rename also overwrites target file
				int err = rename (tempnam.c_str(), fname.c_str());
				if (err != 0)
				{
					perror ("Error renaming file: ");
				}
#endif
				result.fname = fname;
			}
		}
		else
		{
			cout << "Could not open file: " << tempnam << endl;
		}
		
		/* always cleanup */
		curl_easy_cleanup(curl);

		curl_global_cleanup();

	#else
		result.msg = "libCurl not linked, compile with -DUSE_CURL or contact developers";
	#endif
	  return result;
	}

	void run() override {
		cout << "Downloading from :" << url << endl << "File: " << fname << endl;
		Response response = downloadToFile(url, fname);
		cout << "Download Done." << endl;
		cout << response.resultCode << endl << response.msg << endl;
	}

};

shared_ptr<WorkerThread> downloadWorker(const string &url, const string &fname) {
	return make_shared<DownloadWorker>(url, fname);
}

shared_ptr<WorkerThread> postWorker(const string &url, const string &body) {
	return make_shared<PostWorker>(url, body);
}
