# Multithreaded Matrix Mult
A program created for the University of Washington, Tacoma TCSS 422 - Operating Systems by Douglas Johnston and Kevin Tran

## Description
Uses the producer-consumer pattern to perform matrix multiplications

## How to Run
Run the Makefile, then run pcmatrix with arguments in this order:
1. The number of worker threads
2. The size of the bunded buffer
3. The number of matrices to produce/consume
4. The size 1-n of the matrices, or 0 for random matrices

Here's an example command to run pcmatrix
./pcmatrix 2 100 30 0
