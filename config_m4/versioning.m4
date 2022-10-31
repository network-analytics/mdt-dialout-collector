# Set application version based on the git version

#Default
MY_PROJECT_VERSION="$PACKAGE_VERSION" #Unknown (no GIT repository detected)"
FILE_VERSION=`cat $srcdir/VERSION`

AC_CHECK_PROG(ff_git,git,yes,no)

#Normalize
MY_PROJECT_VERSION_NORMALIZED=`echo $MY_PROJECT_VERSION | sed s/dev//g | sed s/RC.*//g | tr -d v`

#Substs
AC_SUBST([MY_PROJECT_VERSION], ["$MY_PROJECT_VERSION"])
AC_SUBST([MY_PROJECT_VERSION_NORMALIZED], ["$MY_PROJECT_VERSION_NORMALIZED"])

AC_MSG_CHECKING([the build version])
AC_MSG_RESULT([$MY_PROJECT_VERSION ($MY_PROJECT_VERSION_NORMALIZED)])

AC_MSG_CHECKING([the build number])
if test $ff_git = no
then
	AC_MSG_RESULT([git not found!])
else

	if test -d $srcdir/.git ; then
		#Try to retrieve the build number
		_MY_PROJECT_GIT_BUILD=`git log -1 --pretty=%H`
		_MY_PROJECT_GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
		_MY_PROJECT_GIT_DESCRIBE=`git describe --abbrev=40`

		AC_SUBST([MY_PROJECT_BUILD], ["$_MY_PROJECT_GIT_BUILD"])
		AC_SUBST([MY_PROJECT_BRANCH], ["$_MY_PROJECT_GIT_BRANCH"])
		AC_SUBST([MY_PROJECT_DESCRIBE], ["$_MY_PROJECT_GIT_DESCRIBE"])

	fi

	AC_MSG_RESULT([$_MY_PROJECT_GIT_BUILD])
fi
