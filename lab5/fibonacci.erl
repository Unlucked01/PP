#!/usr/bin/env escript

main([A]) ->
    I = list_to_integer(A),
    F = fib(I),
    io:format("fibonacci(~w) = ~w~n", [I, F]).

fib(0) -> 0;
fib(1) -> 1;
fib(N) -> 
	fib(N-1) + fib(N-2).

