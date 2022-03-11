# ARM assembly to HEX file converter

Using the standard GNU ARM assembler for doing this.  You will need
the arm2hex Python code on the repository to run this code.  

For 64-bit linux use:

    ./arm2hex arm-none-eabi-as input_file.s output_file.x

arm3hex is slightly modified from arm2hex in order to output program
in byte addressable form for MGC ModelSim.  The syntax is the same as
arm2hex.


