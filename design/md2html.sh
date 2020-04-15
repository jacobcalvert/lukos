#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
rm -rf html
mkdir html
for filename in ./*.md; do
	echo "Processing $filename"
	echo "<html><style>" > html/$filename.html;
	cat $DIR/style.css >> html/$filename.html
	echo "</style>" >> html/$filename.html
	echo "<h1> Table of Contents </h1>" >>  html/$filename.html
	markdown2 -x tables -x fenced-code-blocks -x toc $filename >> html/$filename.html
	echo "</html>" >> html/$filename.html
done
