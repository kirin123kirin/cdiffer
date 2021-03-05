#!/usr/bin/env python
import os, sys
from glob import glob
from os.path import dirname, exists, join as pjoin

def is_manylinux1():
    # from PEP 513 https://www.python.org/dev/peps/pep-0513/#id50
    #
    # Only Linux, and only x86-64 / i686
    from distutils.util import get_platform
    if get_platform() not in ["linux-x86_64", "linux-i686"]:
        return False

    # Check for presence of _manylinux module
    try:
        import _manylinux
        return bool(_manylinux.manylinux1)
    except (ImportError, AttributeError):
        # Fall through to heuristic check below
        pass

    # Check glibc version. CentOS 5 uses glibc 2.5.
    return have_glibc(2, 5)

def have_glibc(major, minimum_minor):
    # from PEP571 https://www.python.org/dev/peps/pep-0571/
    import ctypes

    process_namespace = ctypes.CDLL(None)
    try:
        gnu_get_libc_version = process_namespace.gnu_get_libc_version
    except AttributeError:
        # Symbol doesn't exist -> therefore, we are not linked to
        # glibc.
        return False

    # Call gnu_get_libc_version, which returns a string like "2.5".
    gnu_get_libc_version.restype = ctypes.c_char_p
    version_str = gnu_get_libc_version()
    # py2 / py3 compatibility:
    if not isinstance(version_str, str):
        version_str = version_str.decode("ascii")

    # Parse string and check against requested version.
    version = [int(piece) for piece in version_str.split(".")]
    assert len(version) == 2
    if major != version[0]:
        return False
    if minimum_minor > version[1]:
        return False
    return True


def is_manylinux2010():
    # Only Linux, and only x86-64 / i686
    from distutils.util import get_platform
    if get_platform() not in ["linux-x86_64", "linux-i686"]:
        return False

    # Check for presence of _manylinux module
    try:
        import _manylinux
        return bool(_manylinux.manylinux2010)
    except (ImportError, AttributeError):
        # Fall through to heuristic check below
        pass

    # Check glibc version. CentOS 6 uses glibc 2.12.
    # PEP 513 contains an implementation of this function.
    return have_glibc(2, 12)


def is_manylinux2014():
    # from PE599 https://www.python.org/dev/peps/pep-0599/
    # Only Linux, and only supported architectures
    from distutils.util import get_platform

    if get_platform() not in [
        "linux-x86_64",
        "linux-i686",
        "linux-aarch64",
        "linux-armv7l",
        "linux-ppc64",
        "linux-ppc64le",
        "linux-s390x",
    ]:
        return False

    # Check for presence of _manylinux module
    try:
        import _manylinux

        return bool(_manylinux.manylinux2014)
    except (ImportError, AttributeError):
        # Fall through to heuristic check below
        pass

    # Check glibc version. CentOS 7 uses glibc 2.17.
    # PEP 513 contains an implementation of this function.
    return have_glibc(2, 17)


def main():
    after = None
    if os.name == "posix":
        if is_manylinux2014():
            after = "-manylinux2014_"
        elif is_manylinux2010():
            after = "-manylinux2010_"
        elif is_manylinux1():
            after = "-manylinux1_"
    else:
        return

    if after:
        before = "-linux_"
        for f in glob(sys.argv[1]):
            if f.lower().endswith(".whl") and before in f:
                dst = f.replace(before, after)
                print("Rename whl... {} -> {}".format(f, dst))
                os.rename(f, dst)


if __name__ == "__main__":
    main()