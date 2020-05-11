# Correlation_Coefficient_in_C
Pearson Correlation Coefficient estimate in C, optimized for speed

This code is created for resource limited micro-controllers. The algorithm is based on the <a href=https://en.wikipedia.org/wiki/Pearson_correlation_coefficient>Pearson correlation coefficient</a>.

The approach assumes that x is predetermined, and unchanging, and thus uses some dead code elimination (DCE) to speed up the calculation.

Since x and y are unsigned integers, the size of x was chosen to be divisible by a power of two (32 in this case) so that instead of dividing the sum of the elements, by the size of the arrays (i.e. 32) to get the average, the sum could instead be shifted by 5, thus speeding up the calculation.

Since only one element in the array is changing for each sample, E[Y^2] - [E[Y]]^2 has a fairly narrow boundary from one sample to the next, so an expedient square root estimate is used instead of the square root function that would be found in a typical math.c library. The square root estimate is based on the <a href=https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Babylonian_method>Babylonian method</a>. Since the variation from one sample to the next has a narrow boundary, my initial estimate for the current sample is always the final estimate of the previous sample.

Since in my application, I know that y is a 10 bit unsigned integer, I always iterate the estimate exactly three times, and never check for convergence, because I know that the estimate will be good enough after three iterations. I also force the square root result to be 1 whenever the outcome would otherwise be zero. Otherwise, not only would there be a division by zero, in the final calculation of p(X,Y), the initial estimate for the next sample, and thus all subsequent iterations of the estimate would be zero.
