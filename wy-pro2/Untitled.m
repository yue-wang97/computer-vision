function main
h1 = myfspecial(5,1.5)
h2 = fspecial('gaussian', 5,1.5)
end

function h = generating_kernel(p2, p3)
     siz   = (p2-1)/2;
     std   = p3;
     
     [x,y] = meshgrid(-siz:siz,-siz:siz);
     arg   = -(x.*x + y.*y)/(2*std*std);

     h     = exp(arg);
     h(h<eps*max(h(:))) = 0;

     sumh = sum(h(:));
     if sumh ~= 0,
       h  = h/sumh;
     end;
end
