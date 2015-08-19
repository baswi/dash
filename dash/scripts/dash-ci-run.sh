#!/bin/sh

BASEPATH=`git rev-parse --show-toplevel`
CMD_DEPLOY=$BASEPATH/dash/scripts/dash-ci-deploy.sh
CMD_TEST=$BASEPATH/dash/scripts/dash-test.sh
FAILED=false
TIMESTAMP=`date +%Y%m%d-%H%M%S`

function run_ci
{
  BUILD_TYPE=${1}
  DEPLOY_PATH=$BASEPATH/build-ci/$TIMESTAMP/${BUILD_TYPE}
  mkdir -p $DEPLOY_PATH && \
  cd $DEPLOY_PATH
    echo "[ BUILD  ] Deploying build $BUILD_TYPE to $DEPLOY_PATH ..."
    $CMD_DEPLOY "--b=$BUILD_TYPE" -f "--i=$DEPLOY_PATH" >> build.log 2>&1
    if [ "$?" = "0" ]; then
      echo -n "[ TEST   ] Running tests on build $BUILD_TYPE (SHMEM) ..."
      $CMD_TEST shmem $DEPLOY_PATH/bin > test_shmem.log 2>&1
      if [ "$?" = "0" ]; then
        echo " OK"
      else
        echo " FAILED"
      fi
      echo -n "[ TEST   ] Running tests on build $BUILD_TYPE (MPI)   ..."
      $CMD_TEST mpi   $DEPLOY_PATH/bin > test_mpi.log 2>&1
      if [ "$?" = "0" ]; then
        echo " OK"
      else
        echo " FAILED"
      fi
    else
      echo "[ FAIL   ] Build failed, see $DEPLOY_PATH/build.log for details"
      FAILED=true
    fi
  if [ `find $DEPLOY_PATH -name 'dash-test*.log' | xargs cat | grep --count 'FAIL'` -gt 0 ]; then
    echo "[ FAIL   ] Failed tests!"
    FAILED=true
  fi
  if ! $FAILED; then
    echo "[ PASSED ] Build and test suite passed"
  fi
}

run_ci Release
run_ci Debug

if $FAILED; then
  exit -1
fi

