clear all;

AMPLITUDE=1;
FREQUENCY=5;
SAMPLING_RATE=64;

BITS=3;

e=2.7183;

% time scale (always 1 second)
t=[0:1/SAMPLING_RATE:1-1/SAMPLING_RATE];
f=[-SAMPLING_RATE/2:SAMPLING_RATE/2-1];


% define oscillation here
% =======================

% complex oscillation
f_t=(AMPLITUDE.*e.^(i*2*pi*FREQUENCY*t));
% sinus
%f_t=real(f_t);
% cosinus
f_t=-imag(f_t);

% Quantization
% ============

f_t=round(f_t.*(2^BITS));

% fft
F_f=fft(f_t);
F_f=fftshift(F_f);

% display
subplot(2,1,1);
plot(t,real(f_t),'b');
hold on;
plot(t,imag(f_t),'r');
hold off;
grid on;
subplot(2,1,2);
plot(f,real(F_f),'b');
hold on;
plot(f,imag(F_f),'r');
hold off;
grid on;

% Data dump
% =========

fd_time=fopen('cosinus_time.txt','W');
fd_freq=fopen('cosinus_freq.txt','W');

for i=1:SAMPLING_RATE
    
    fprintf(fd_time,'%d + i%d\n',real(f_t(i)),imag(f_t(i)));
    fprintf(fd_freq,'%d + i%d\n',real(F_f(i)),imag(F_f(i)));
    
end

fclose(fd_time);
fclose(fd_freq);