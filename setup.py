#!/usr/bin/env python
try:
    from setuptools import setup, Extension
except:
    from distutils.core import setup, Extension

ext_modules = [Extension('cdiffer',
        sources = ['cdiffer.c'],
        #include_dirs=['.'],
        )]

setup(name="cdiffer",
    version='0.0.2',
    description="Usefull differ function with Levenshtein distance.",
    long_description='',
    author='kirin123kirin',
    ext_modules=ext_modules,
    classifiers=['License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',],)
