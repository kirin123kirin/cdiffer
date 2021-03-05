#!/usr/bin/env python

from setuptools import setup, Extension

import codecs
from os.path import dirname, exists, join as pjoin

def read(fname1="README.rst", fname2="README.md"):
    if exists(fname1):
        return codecs.open(pjoin(dirname(__file__), fname1), encoding="utf-8").read()
    if exists(fname2):
        return codecs.open(pjoin(dirname(__file__), fname2), encoding="utf-8").read()
    return ""


__version__ = '0.0.7'


ext_modules = [Extension('cdiffer',
        sources = ['cdiffer.c'],
        #include_dirs=['.'],
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

setup(name="cdiffer",
    version=__version__,
    description="Usefull differ function with Levenshtein distance.",
    long_description_content_type='text/markdown',
    long_description=codecs.open(pjoin(dirname(__file__), "README.md"), encoding="utf-8").read(),
    url='https://github.com/kirin123kirin/cdiffer',
    author='kirin123kirin',
    ext_modules=ext_modules,
    keywords = "diff, comparison, compare",
    license="GPL2",
    platforms=["Windows", "Linux", "Mac OS-X", "Unix"],
    classifiers=CF.strip().splitlines(),
    )
