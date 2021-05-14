DIRNAME = ep1-lucas_paiolla
EPOBJS = ep1.o util.o headers.o topics.o
CFLAGS = -Wall -g -pedantic -Wno-unused-result -fsanitize=address
COMPILER = gcc

default : ep1

ep1 : $(EPOBJS)
	$(COMPILER) $(CFLAGS) $(EPOBJS) -o ep1

%.o: %.c
	$(COMPILER) $(CFLAGS) -c $<

tar:
	rm -Rf ../$(DIRNAME)
	rm -f ../$(DIRNAME).tar.gz
	mkdir ../$(DIRNAME)

	# copia os slides, o LEIAME e o Makefile
	cp ../LEIAME ../$(DIRNAME)/LEIAME
	cp Makefile ../$(DIRNAME)/Makefile
	cp ../slides/slides.pdf ../$(DIRNAME)/slides.pdf

	# copia o codigo fonte
	for a in *.cpp; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# (se houverem .h)
	for a in *.h; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# gera ../$(DIRNAME).tar.gz
	tar -czvf ../$(DIRNAME).tar.gz ../$(DIRNAME)
	rm -Rf ../$(DIRNAME)


clean:
	rm *.o ep1
