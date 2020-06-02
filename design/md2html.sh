#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
rm -rf html
mkdir html
for filename in ./*.md; do
	echo "Processing $filename"
	echo "<html><head><style>" > html/$filename.html;
	cat $DIR/style.css >> html/$filename.html
	echo "</style>" >> html/$filename.html
	echo "<link rel='stylesheet' href='http://cdnjs.cloudflare.com/ajax/libs/highlight.js/10.0.3/styles/default.min.css'>" >> html/$filename.html
	echo "<script src='http://cdnjs.cloudflare.com/ajax/libs/highlight.js/10.0.3/highlight.min.js'></script>" >> html/$filename.html
	markdown2 -x tables -x fenced-code-blocks -x highlightjs-lang $filename >> html/$filename.html
	echo "</html>" >> html/$filename.html
	sed -i 's/\(\w.md\)/\1.html/g'  html/$filename.html
done
