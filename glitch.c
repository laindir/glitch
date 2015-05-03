#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

uint32_t stack[256];
uint32_t t;
uint8_t tosp;

void
push(uint32_t value)
{
	stack[++tosp] = value;
}

uint32_t
pop(void)
{
	return stack[tosp--];
}

void
noop(void)
{
}

void
T(void)
{
	push(t);
}

void
PUT(void)
{
	uint32_t v1 = stack[tosp];
	uint32_t v2 = v1 & 0xff;
	uint32_t v3 = v2 + 1;
	uint32_t v4 = stack[(tosp + 1) & 0xff];
	stack[(0x100 + tosp - v3) & 0xff] = v4;
	(void)pop();
}

void
DROP(void)
{
	(void)pop();
}

void
MUL(void)
{
	push(pop() * pop());
}

void
DIV(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	uint32_t v3 = v1 ? v2 / v1 : 0;
	push(v3);
}

void
ADD(void)
{
	push(pop() + pop());
}

void
SUB(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	push(v2 - v1);
}

void
MOD(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	uint32_t v3 = v1 ? v2 % v1 : 0;
	push(v3);
}

void
LSHIFT(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	uint32_t s;
	if(v2 == 0)
	{
		push(0);
		return;
	}
	for(s = 0; (1U << (31 - s)) > v2; s++) { }
	if(v1 > s)
	{
		push(0);
	}
	else
	{
		push(v2 << v1);
	}
}

void
RSHIFT(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	push(v2 >> v1);
}

void
AND(void)
{
	push(pop() & pop());
}

void
OR(void)
{
	push(pop() | pop());
}

void
XOR(void)
{
	push(pop() ^ pop());
}

void
NOT(void)
{
	push(~pop());
}

void
DUP(void)
{
	uint32_t v1 = pop();
	push(v1);
	push(v1);
}

void
PICK(void)
{
	uint32_t v1 = stack[tosp];
	uint32_t v2 = v1 + 1;
	uint32_t v3 = v2 & 0xff;
	uint32_t v4 = stack[(0x100 + tosp - v3) & 0xff];
	(void)pop();
	push(v4);
}

void
SWAP(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	push(v1);
	push(v2);
}

void
LT(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	if(v2 < v1)
	{
		push(0xffffffff);
	}
	else
	{
		push(0);
	}
}

void
GT(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	if(v2 > v1)
	{
		push(0xffffffff);
	}
	else
	{
		push(0);
	}
}

void
EQ(void)
{
	uint32_t v1 = pop();
	uint32_t v2 = pop();
	if(v2 == v1)
	{
		push(0xffffffff);
	}
	else
	{
		push(0);
	}
}

void (*opcodes[256])(void);

void
standard_opcodes(void)
{
	opcodes['a'] = T;
	opcodes['b'] = PUT;
	opcodes['c'] = DROP;
	opcodes['d'] = MUL;
	opcodes['e'] = DIV;
	opcodes['f'] = ADD;
	opcodes['g'] = SUB;
	opcodes['h'] = MOD;
	/*no i*/
	opcodes['j'] = LSHIFT;
	opcodes['k'] = RSHIFT;
	opcodes['l'] = AND;
	opcodes['m'] = OR;
	opcodes['n'] = XOR;
	opcodes['o'] = NOT;
	opcodes['p'] = DUP;
	opcodes['q'] = PICK;
	opcodes['r'] = SWAP;
	opcodes['s'] = LT;
	opcodes['t'] = GT;
	opcodes['u'] = EQ;
}

void
alternate_opcodes(void)
{
	opcodes['.'] = T;
	opcodes[','] = PUT;
	opcodes['_'] = DROP;
	opcodes['*'] = MUL;
	opcodes['/'] = DIV;
	opcodes['+'] = ADD;
	opcodes['-'] = SUB;
	opcodes['%'] = MOD;
	opcodes['('] = LSHIFT;
	opcodes[')'] = RSHIFT;
	opcodes['&'] = AND;
	opcodes['|'] = OR;
	opcodes['^'] = XOR;
	opcodes['~'] = NOT;
	opcodes['"'] = DUP;
	opcodes['\''] = PICK;
	opcodes['$'] = SWAP;
	opcodes['<'] = LT;
	opcodes['>'] = GT;
	opcodes['='] = EQ;
}

void
exec(unsigned char c)
{
	void (*op)(void) = opcodes[c];
	op ? op() : noop();
}

void
parse(char *s)
{
	int n = 0;
	int a = 0;
	while(*s)
	{
		if('0' <= *s && *s <= '9')
		{
			n = 1;
			a *= 16;
			a |= *s - '0';
		}
		else if('A' <= *s && *s <= 'F')
		{
			n = 1;
			a *= 16;
			a |= *s - 'A' + 10;
		}
		else if(n == 1)
		{
			push(a);
			n = 0;
			a = 0;
			exec(*s);
		}
		else
		{
			exec(*s);
		}
		s++;
	}
}

struct opts
{
	int alt_ops;
};

struct opts
parse_opts(int argc, char *argv[])
{
	const char options[] = "a";
	struct opts result = {0};
	int opt;
	while((opt = getopt(argc, argv, options)) != -1) switch(opt)
	{
	case 'a':
		result.alt_ops = 1;
		break;
	case '?':
		exit(EXIT_FAILURE);
		break;
	}
	return result;
}

char audio[4096];
char buf[100];

void
getaudio(void)
{
	int i;
	for(i = 0; i < 4096; i++)
	{
		parse(buf);
		audio[i] = stack[tosp];
		t++;
	}
}

int
main(int argc, char *argv[])
{
	int efd = epoll_create(1);
	struct epoll_event evt = {EPOLLIN, {STDIN_FILENO}};
	int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
	struct itimerspec tmr = {{0, 512000000}, {2, 48000000}};
	int running = 1;
	uint64_t ticks = 0;
	struct opts options = parse_opts(argc, argv);
	options.alt_ops ? alternate_opcodes() : standard_opcodes();
	epoll_ctl(efd, EPOLL_CTL_ADD, STDIN_FILENO, &evt);
	evt.data.fd = tfd;
	epoll_ctl(efd, EPOLL_CTL_ADD, tfd, &evt);
	timerfd_settime(tfd, 0, &tmr, NULL);
	while(running)
	{
		epoll_wait(efd, &evt, 1, -1);
		if(evt.data.fd == STDIN_FILENO)
		{
			if(fgets(buf, 100, stdin))
			{
				for(t = 0; t < 256; t++)
				{
					stack[t] = 0;
				}
				tosp = 0;
				t = 0;
				getaudio();
			}
			else
			{
				break;
			}
		}
		else if(evt.data.fd == tfd)
		{
			read(tfd, &ticks, sizeof(ticks));
			write(STDOUT_FILENO, audio, 4096);
			getaudio();
		}
	}
	return 0;
}
