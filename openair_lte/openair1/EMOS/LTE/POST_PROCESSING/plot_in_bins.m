function [out,n] = plot_in_bins(x,y,edges)
%  [out] = plot_in_bins(data,bins)
% This function puts the data y = y(x) in bins of x. For every bin it calculates average,
% 50%, 90% and 95% percentiles. The output is a matrix with 4 columns
% corresponding to the computed statistics and length(edges) row. See help
% histc for treatment of edges.

[n, bin] = histc(x,edges);

out = zeros(length(edges),4);

for k=1:length(edges)
    out(k,1) = mean(y(bin==k));
    out(k,2:4) = prctile(y(bin==k),[50 90 95]);
end

bar(edges,out);
for i=1:length(edges)
    text(edges(i),max(out(i,:)),num2str(n(i)), 'HorizontalAlignment','left','VerticalAlignment','middle','Rotation',90);
end
legend('Mean','50% percentile','90% percentile','95% percentile');