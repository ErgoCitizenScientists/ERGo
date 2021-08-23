import setuptools

with open("README.md", "r") as f:
    long_description = f.read()

setuptools.setup(
    name='erg',
    version='0.0.1',
    author='Daniel Pollak',
    author_email='danpollak23@gmail.com',
    description='Package for unpacking erg data.',
    long_description=long_description,
    long_description_content_type='ext/markdown',
    packages=setuptools.find_packages(),
    install_requires=["numpy","pandas","scipy","matplotlib","holoviews","bokeh>=1.4.0"],
    classifiers=(
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ),
)