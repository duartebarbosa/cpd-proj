DOC = cpd-report

all: clean pdf

pdf: $(DOC).tex
	pdflatex $(DOC)
	pdflatex $(DOC)

clean:
	$(RM) $(DOC).pdf *.log *.aux *.bbl *.blg
	
.PHONY: clean
