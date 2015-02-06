NAME = tp-description
all: $(NAME).pdf clean

%.pdf: %.aux
	pdflatex $*.tex
%.aux: %.tex
	pdflatex $*.tex

clean:
	$(RM) $(NAME).log $(NAME).aux

realclean: clean
	$(RM) $(NAME).pdf
