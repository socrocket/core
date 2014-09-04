revision=`bzr version-info --custom --template '{revno}'`

all: dist


dist: DoxygenPreprocessor/doxygen_preprocessor.exe
	
	bzr export doxygen_preprocessor-r${revision}.tar.gz
	zip -r doxygen_preprocessor-r${revision}.zip DoxygenPreprocessor
	mkdir -p dist
	rm -rf dist/*
	mv doxygen_preprocessor-r${revision}* dist

DoxygenPreprocessor/doxygen_preprocessor.exe: *.py
	wpython setup.py py2exe
