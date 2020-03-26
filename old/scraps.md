[create an anchor](#anchors-in-markdown)

nn = 1;
for kk = 1:length(frqs)
    if (frqs(kk) > 90 && bw(kk) <400)
        formants(nn) = frqs(kk);
        bws(nn) = bw(kk);
        nn = nn+1;
    end
end
formants
bws
