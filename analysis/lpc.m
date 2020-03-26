## Copyright (C) 2013 Leonardo Araujo
##
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free Software
## Foundation; either version 3 of the License, or (at your option) any later
## version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {[@var{a}, @var{g}] = } lpc (@var{x}, @var{p})
## @deftypefnx {Function File} {[@var{a}, @var{g}] = } lpc (@var{x})
## @deftypefnx {Function File} {@var{a} = } lpc (@var{x}, @var{p})
## @deftypefnx {Function File} {@var{a} = } lpc (@var{x})
##
## Determines the forward linear predictor by minimizing the prediction error
## in the least squares sense. Use the Durbin-Levinson algorithm to solve
## the Yule-Walker equations obtained by the autocorrelation of the input 
## signal.
##
## @var{x} is a data vector used to estimate the lpc model of @var{p}-th order,
## given by the prediction polynomial @code{@var{a} = [1 @var{a}(2) ... 
## @var{a}(@var{p}+1)]}. If @var{p} is not provided, @code{length(@var{p}) - 1} 
## is used as default.
## 
## @var{x} might also be a matrix, in which case each column is regarded as a
## separate signal. @code{lpc} will return a model estimate for each column of 
## @var{x}.
##
## @var{g} is the variance (power) of the prediction error for each signal in 
## @var{x}.
## 
## @end deftypefn
## @seealso{aryule,levinson}

function [a, g] = lpc (x, p)
  if (nargin < 1 || nargin > 2)
    print_usage;
  elseif (! isrow (x) && rows (x) < 2)
    error( "lpc: rows(X) must be >1" );
  elseif (nargin == 2 && (! isscalar (p) || fix (p) != p ||  p > length (x) - 1
           || p < 1))
    error( "lpc: P must be an integer >0 and <rows(X)" );
  endif

  if (nargin < 2)
    p = length (x) - 1;
  endif

  if (isrow (x))
    x = x';
  endif

  [m, n] = size(x);

  for j = 1:n
    r = xcorr (x(:,j)', p+1, 'biased');
    r(1:p+1) = [];            # remove negative autocorrelation lags
    r(1) = real (r(1));       # levinson/toeplitz requires exactly real r(1)
    [a(j,:), g(j,1)] = levinson (r, p); # Use the Durbin-Levinson to solve:
                              #    toeplitz (acf(1:p)) * x = -acf(2:p+1).
  endfor

endfunction


%!demo
%! noise = randn (10000, 1);
%! x = filter (1, [1 1/2 1/4 1/8], noise);
%! x = x(end-4096:end);
%! [a, g] = lpc (x, 3);
%! xe = filter ([0 -a(2:end)], 1, x);
%! e = x - xe;
%! [ac, k] = xcorr (e, "coeff");
%! figure (1); plot (x(1:100), "b-", xe(1:100), "r--");
%! xlabel ("sample"); ylabel ("amplitude"); legend ("original","LPC estimate");
%! figure (2); plot(k,ac,"b-"); xlabel ("lag"); 
%! title ("autocorrelation of prediction error");

%!demo
%! if !isempty ( pkg ("list", "ltfat") )
%!   pkg load ltfat
%!   [sig, fs] = linus;
%!   x = sig(13628:14428);
%!   [a, g] = lpc (x, 8);
%!   F = round (sort (unique (abs (angle (roots (a))))) * fs / (2 * pi) );
%!   [h, w] = freqz (1, a, 512, "whole");
%!   subplot (2, 1, 1);
%!   plot ( 1E3 * [0:1/fs:(length(x)-1)*1/fs], x);
%!   xlabel ("time (ms)"); ylabel ("Amplitude");
%!   title ( "'linus' test signal" );
%!   subplot (2, 1, 2);
%!   plot (w(1:256)/pi, 20*log10 (abs (h(1:256))));
%!   xlabel ('Normalized Frequency (\times\pi rad/sample)')
%!   ylabel ("Magnitude (dB)")
%!   txt = sprintf("Signal sampling rate = %d kHz\nFormant frequencies:\ 
%!     \nF1 = %d Hz\nF2 = %d Hz\nF3 = %d Hz\nF4 = %d Hz", fs/1E3, F(1), F(2), \
%!     F(3), F(4));
%!   text (0.6, 20, txt);
%! endif

%! ## test input validation
%!error [a, g] = lpc ()
%!error [a, g] = lpc (1)
%!error [a, g] = lpc (1, 1)
%!error [a, g] = lpc (1, 1, 1)
%!error [a, g] = lpc (1:10, 0)
%!error [a, g] = lpc (1:10, 10)
%!error [a, g] = lpc (1:10, 0.5)
%!error [a, g] = lpc (1:10, 1, [1 2])

%!test
%! x = [1:4 4:-1:1];
%! [a, g] = lpc (x, 5);
%! assert (a, [1.0  -1.823903  1.101798  -0.405738  0.521153  -0.340032], 1e-6)
%! assert (g, 0.272194, 1e-6)
