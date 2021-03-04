#!/usr/bin/env python

try:
    from setuptools import setup, Extension
except:
    from distutils import setup, Extension

ext_modules = [
    Extension(
        'cdiffer.bycython',
        ['_cdiffer.c'],
        include_dirs=['.'],
    )
]

setup(
    name="cdiffer",
    version='0.0.1',
    description="Usefull differ function with Levenshtein distance",
    long_description='',
    author='kirin123kirin',
    ext_modules=ext_modules,
    packages=['cdiffer'],
    package_data={'cdiffer': ['_cdiffer.h']},
    classifiers=[
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
    ],
)