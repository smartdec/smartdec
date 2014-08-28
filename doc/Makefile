TXT_FILES	= build.txt hacking.txt licenses.txt
HTML_FILES	= $(TXT_FILES:.txt=.html)

.PHONY: all
all: $(HTML_FILES)

%.html: %.txt
	asciidoc -b html -o $@ $<

.PHONY: clean
clean:
	rm -f $(HTML_FILES)
