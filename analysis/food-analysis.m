food = wavread("foooood.wav")

pkg load signal

specgram(food,segmentlen,noverlap,NFFT,44100,'yaxis')

Fs =  44100

x1 = food.*hamming(length(food));

preemph = [1 0.63];
x1 = filter(1,preemph,x1);

A = lpc(x1,8);
rts = roots(A);

rts = rts(imag(rts)>=0);
angz = atan2(imag(rts),real(rts));

[frqs,indices] = sort(angz.*(Fs/(2*pi)));
bw = -1/2*(Fs/(2*pi))*log(abs(rts(indices)));

nn = 1;
for kk = 1:length(frqs)
    if (frqs(kk) > 90 && bw(kk) <5000)
        formants(nn) = frqs(kk);
        bws(nn) = bw(kk);
        nn = nn+1;
    end
end

formants
bws
