function [ pmi G ] = compute_pmi( H, simparms )
%COMPUTE_PMI Summary of this function goes here
%   Detailed explanation goes here

G0 = simparms.CB(:,1:2);
G1 = simparms.CB(:,3:4);

[~, pmi] = min([getMinD(H*G0, simparms), getMinD(H*G1, simparms)]);
pmi = pmi-1;

if (pmi)
	G = G1;
else
	G = G0;	
end

end

function dmin = getMinD(H, simparms)

% Holds all distances
dmin = Inf;

% Compute all distances
for k0=1:length(simparms.C)
	s0 = simparms.C(:,k0);
	for k1=1:length(simparms.C)
		s1 = simparms.C(:,k1);
		if (~sum(s0==s1))
			dmin = min([dmin, norm(H*(s0 - s1),2)]);
		end
	end
end

end

