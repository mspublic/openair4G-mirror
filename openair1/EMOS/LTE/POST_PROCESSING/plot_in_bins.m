function [out,n,n2] = plot_in_bins(x,y,edges)
%  [out] = plot_in_bins(data,bins)
% This function puts the data y = y(x) in bins of x. For every bin it calculates average,
% 50%, 90% and 95% percentiles. The output is a matrix with 4 columns
% corresponding to the computed statistics and length(edges) row. See help
% histc for treatment of edges.

if (length(x) ~= length(y))
    error('x and y must be the same size!');
end
x=x(:);
y=y(:);

[n, bin] = histc(x,edges);

out = zeros(length(edges),4);
n2 = zeros(size(n));

for k=1:length(edges)
    n2(k) = sum(bin==k  & ~isnan(y));
    out(k,1) = mean(y(bin==k & ~isnan(y)));
    out(k,2:4) = prctile(y(bin==k & ~isnan(y)),[50 90 95]);
end

plot(edges(1:end-1),out(1:end-1,:));
% for i=1:length(edges)-1
%     %text(edges(i),max(out(i,:)),sprintf('%d (%d %%)',n2(i),round(n2(i)/n(i))), 'HorizontalAlignment','left','VerticalAlignment','middle','Rotation',90);
%     text(edges(i),0,sprintf('%d (%d %%)',n2(i),round(100*n2(i)/n(i))), 'HorizontalAlignment','left','VerticalAlignment','middle','Rotation',90);
% end
legend('Mean','50% prctile below','90% prctile below','95% prctile below','Location','Best');