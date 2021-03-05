#!/usr/bin/env python

try:
    from setuptools import setup, Extension
except:
    from distutils.core import setup, Extension

import codecs
from os.path import dirname, exists, join as pjoin

def read(fname="README.md"):
    if exists(fname):
        return codecs.open(pjoin(dirname(__file__), fname), encoding="utf-8").read()
    return ""


__version__ == '0.0.4'


ext_modules = [Extension('cdiffer',
        sources = ['cdiffer.c'],
        #include_dirs=['.'],
        )]

# Development Status
#   3 - Alpha
#   4 - Beta
#   5 - Production/Stable
CF = """
Development Status :: 4 - Beta
Intended Audience :: Developers
Operating System :: OS Independent
Topic :: Software Development :: Libraries :: Python Modules
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
Operating System :: Microsoft :: Windows
Operating System :: POSIX
Operating System :: Unix
Operating System :: MacOS
"""

setup(name="cdiffer",
    version=__version__,
    description="Usefull differ function with Levenshtein distance.",
    long_description_content_type='text/markdown',
    long_description=read(),
    url='https://github.com/kirin123kirin/cdiffer',
    author='kirin123kirin',
    ext_modules=ext_modules,
    keywords = "diff, comparison, compare",
    license="GPL2",
    platforms=["Windows", "Linux", "Mac OS-X", "Unix"],
    classifiers=CF.strip().splitlines(),
    )
