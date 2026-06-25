target := app
OUT := layout.ppm
SRC := main.c canvas.c point.c layout.c
VIEWER := feh

.PHONY: clean, run

$(target): $(SRC)
	gcc -ggdb -lm -o $@ $^

run: $(target)
	./$(target)

view: run
	$(VIEWER) $(OUT)

clean:
	rm -f *.o $(target) $(OUT)
