#!/usr/bin/env python
import re

from setuptools import Extension, setup
import os
import io
import sys
from os.path import dirname, join as pjoin

__version__ = '0.1.6'

# Edit posix platname for pypi upload error
if os.name == "posix" and any(x.startswith("bdist") for x in sys.argv) \
        and not ("--plat-name" in sys.argv or "-p" in sys.argv):

    if "64" in os.uname()[-1]:
        from platname import get_platname_64bit

        plat = get_platname_64bit()
    else:
        from platname import get_platname_32bit

        plat = get_platname_32bit()
    sys.argv.extend(["--plat-name", plat])

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

# Readme read or edit
readme = pjoin(dirname(__file__), "README.md")
badge = re.compile(r'(\[!\[.*?\]\(https://.*?badge\.(?:svg|png)\?branch=([^\)]+)\)\])')
description = ""
is_change = False
with io.open(readme, encoding="utf-8") as f:
    for line in f:
        res = badge.search(line)
        if res and __version__ not in res.group(2):
            for b, k in badge.findall(line):
                line = line.replace(b, b.replace(k, "v" + __version__))
            is_change = True
        description += line

if is_change:
    with io.open(readme, "w", encoding="utf-8") as f:
        f.write(description)

# for python2.7
tests = {}
if sys.version_info[:2] >= (3, 3):
    tests = dict(
        setup_requires=["pytest-runner"],
        tests_require=["pytest", "pytest-cov"])

setup(name="cdiffer",
      version=__version__,
      description="Usefull differ function with Levenshtein distance.",
      long_description_content_type='text/markdown',
      long_description=description,
      url='https://github.com/kirin123kirin/cdiffer',
      author='kirin123kirin',
      ext_modules=ext_modules,
      keywords=["diff", "comparison", "compare"],
      license="GPL2",
      platforms=["Windows", "Linux"],  # , "Mac OS-X", "Unix"
      classifiers=CF.strip().splitlines(),
      **tests
      )
