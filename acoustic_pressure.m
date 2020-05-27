c = 343; % sound speed
nu = 40000; % wave frequency
lambda = c / nu;

P0 = 0.1; % amplitude constant that defines the transducer output power
V = 12; % exitation signal peak-to-peak amplitude
theta = 0; % angle between piston normal and r
phi = 0; % emitting phase of the source
k = 2*pi / lambda; % wavenumber
a = 0.0045; % radius of the piston
Df = Df_f(k, a, theta); % directivity function
h_max = 2*lambda;
w_min = -1*lambda;
w_max = 1*lambda;
N = 200;
N_low = 1;
h_a = linspace(0, h_max, N);
w_a = linspace(w_min, w_max, N);
p_a1 = zeros(N, N);
p_a2 = zeros(N, N);
p_a = zeros(N, N);

figure
surfP = surf(w_a(N_low:N), h_a(N_low:N), p_a(N_low:N, N_low:N), 'EdgeColor','none','LineStyle','none','FaceLighting','phong');
zlim([-3000 3000]);
caxis([0 1000]);
colormap(hot);
view(2);

counterx = 1;
countery = 1;

%while ishandle(surfP)
    counterx = 1;
    %phi = phi + pi/12;
    for x = w_a
       countery = 1;
       for y = h_a
           d = sqrt(x*x + y*y);
           theta = atan(x/y);
           
           Df = Df_f(k, a, theta);
           p_a1(countery, counterx) = real(P_f(P0, V, Df, d, phi, k));
           p_a2(countery, counterx) = real(P_f(P0, V, Df, d, -phi, k));
           
           countery = countery + 1;
       end
       counterx = counterx + 1;
    end
    p_a2 = flipud(p_a2);
    p_a = p_a1 + p_a2;
    set(surfP, 'ZData', abs(p_a(N_low:N, N_low:N)));
    %format rat
    %disp(phi/pi);
    pause(0.001);
%end

function Df = Df_f(k, a, theta)
    Df = 2*besselj(1, k*a*sin(theta)) / (k*a*sin(theta));
end
function P = P_f(P0, V, Df, d, phi, k)
    P = P0*V*(Df/d)*exp(1i*(phi + k*d));
end