.PHONY: all
all: main.pdf

images/workflow.pdf: images/workflow.dot
	dot -Tpdf $^ > $@

main.pdf: main.tex images/workflow.pdf
	pdflatex main.tex && pdflatex main.tex

.PHONY:
clean:
	rm -f main.aux main.log main.out main.pdf main.toc
