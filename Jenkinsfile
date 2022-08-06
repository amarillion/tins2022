node {

	catchError {
		def TWIST_HOME = "${env.JENKINS_HOME}/jobs/twist5/workspace"
		
		stage('CheckOut') {
			checkout scm
		}

		stage('Build W32') {
			docker.image('amarillion/alleg5-plus-buildenv:latest-mingw-w64-i686').inside() {
				sh "make all TARGET=CROSSCOMPILE WINDOWS=1 BUILD=RELEASE"
				sh "./gather-dlls.sh"
			}
		}

		stage('Build Linux') {
			docker.image('amarillion/alleg5-plus-buildenv:latest').inside() {
				sh "make all BUILD=STATIC" 
				sh "./gather-so.sh"
			}
		}
		
		stage('Package') {
			
			sh "zipper -f tgz src-linux"
			sh "zipper -f zip win"
			archiveArtifacts artifacts: 'dist/*.tar.gz,dist/*.zip'
			sh "cp dist/*.tar.gz dist/*.zip /srv/smbshare/builds"

		}
	}

//	mailIfStatusChanged env.EMAIL_RECIPIENTS
	mailIfStatusChanged "mvaniersel@gmail.com"
}

//see: https://github.com/triologygmbh/jenkinsfile/blob/4b-scripted/Jenkinsfile
def mailIfStatusChanged(String recipients) {
    
	// Also send "back to normal" emails. Mailer seems to check build result, but SUCCESS is not set at this point.
    if (currentBuild.currentResult == 'SUCCESS') {
        currentBuild.result = 'SUCCESS'
    }
    step([$class: 'Mailer', recipients: recipients])
}
