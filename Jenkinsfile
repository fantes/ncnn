pipeline {
  agent none
  stages {
    stage('Build') {
      steps {
        sh '''mkdir -p build
cd build
cmake ..
make -j24'''
      }
    }
    stage('Benchmarks') {
      steps {
        sh '''cd build/benchmarks
cp -r ../../benchmark/
./benchmarks 8 4 0
'''
      }
    }
    stage('Notify Chat') {
      steps {
        rocketSend(message: 'Build Completed', avatar: 'jenkins', channel: 'dev')
      }
    }
  }
}