#!/usr/bin/env python
from setuptools import Extension, setup
import os
import sys
import codecs
from os.path import dirname, join as pjoin

__version__ = '0.0.9'

try:
    # Edit posix platname for pypi upload error
    if os.name == "posix" and any(x.startswith("bdist") for x in sys.argv) \
            and not ("--plat-name" in sys.argv or "-p" in sys.argv):

        if "64" in os.uname()[-1]:
            from _cross_platform import get_platname_64bit

            plat = get_platname_64bit()
        else:
            from _cross_platform import get_platname_32bit

            plat = get_platname_32bit()
        sys.argv.extend(["--plat-name", plat])
except (ImportError, ModuleNotFoundError):
    pass

ext_modules = [Extension('cdiffer',
                         sources=['cdiffer.c'],
                         # include_dirs=['.'],
                         )]

# Development Status Example
#   3 - Alpha
#   4 - Beta
#   5 - Production/Stable

CF = """
Development Status :: 4 - Beta
License :: OSI Approved :: GNU General Public License v2 (GPLv2)
Programming Language :: C
Programming Language :: Python
Programming Language :: Python :: 2.7
Programming Language :: Python :: 3.5
Programming Language :: Python :: 3.6
Programming Language :: Python :: 3.7
Programming Language :: Python :: 3.8
Programming Language :: Python :: 3.9
Programming Language :: Python :: Implementation :: CPython
Operating System :: OS Independent
Operating System :: Microsoft :: Windows
Operating System :: POSIX
"""
# Not yet
"""
Operating System :: MacOS
Operating System :: Unix
"""

readme = pjoin(dirname(__file__), "README.md")

setup(name="cdiffer",
      version=__version__,
      description="Usefull differ function with Levenshtein distance.",
      long_description_content_type='text/markdown',
      long_description=codecs.open(readme, encoding="utf-8").read(),
      url='https://github.com/kirin123kirin/cdiffer',
      author='kirin123kirin',
      ext_modules=ext_modules,
      keywords=["diff", "comparison", "compare"],
      license="GPL2",
      platforms=["Windows", "Linux"],  # , "Mac OS-X", "Unix"
      classifiers=CF.strip().splitlines(),
      )
