Simulate process communication using Linux functions pipes and fork.
processes are labeled as "p" followed by a process number.
Inputs are labeled as variables and will be set to value in input file respectively.


To run code, run through a terminal.

Make sure to input file names in this order:
1. s file (file with pipe commands)
2. input file (file with integer inputs)
Example: ./main s1.txt input1.txt

There can be a space or commas between input variables and internal variables and should end with semicolon.
For example:
"input_var x,y,z;"
and
"internal_var p0,p1,p2;"

or

"input_var x y z;"
and
"internal_var p0 p1 p2;"

There should also be a space between the pipe commands.
For example:
"x -> p0;"

Overall pipe command input should look like this:
"input_var x,y,z;
internal_var p0,p1,p2;
   x -> p0;
   y -> p1;
 + z -> p1;
   p1 -> p2;
 - p0 -> p2;
write(x, y, z, p0, p1, p2)."

The input file can be separated by spaces or commas.
For example:
"1,2,3"
or
"1 2 3"

If we run the above inputs, we will get:
p0 = x
p1 = y + z
p2 = (y + z) - x

The logic for my code:
I load all the variables into a map along with its value or an empty string.
The reason for using a map is to avoid searching through the array to update variables.
I then load all pipe commands into a string vector. This is the best way to create pipes dynamically.
I create 1 pipe for every internal variable.
I also create 1 fork for every internal variable, but I am careful to only create a process from the main process.
Then the correct process will write the value through its designated pipe to the parent.
The parent will read the value and update the pipe.