# glitch #

glitch is a text based program inspired by [libglitch](https://github.com/erlehmann/libglitch). It uses a forth-like language from stdin, operating on a ring-buffer and a sawtooth wave to produce 8-bit audio on stdout.

## Usage ##

	./glitch >/dev/dsp

## Options ##

	-a	Enable alternate opcodes

## Tips ##

	rlwrap ./glitch >/dev/dsp

## Examples ##

	{ echo '..4)&.4*.7)&|'; sleep 6; echo '..4)&.3*.7)&|1-'; sleep 4; } | ./glitch -a >/dev/dsp
	{ echo '.9*.4)&.5*.7)&|.3*.400/&|1-'; sleep 20; } | ./glitch -a >/dev/dsp
