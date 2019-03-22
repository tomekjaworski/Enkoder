clear;
clc;
%% laczenie i czyszczenie
if (0)
    comm_encoder = serial('com3', 'baudrate', 115200);
    comm_encoder.Timeout = 1;
    comm_encoder.InputBufferSize = 10 * 1024;
else
    comm_encoder = tcpip('192.168.0.19', 5001);
    comm_encoder.Timeout = 1;
    comm_encoder.InputBufferSize = 10 * 1024;
end;

fopen(comm_encoder);

fprintf('Uruchamianie kontrolera enkodera...\n');
fwrite(comm_encoder, ['+++' char(10)]);
pause(0.2);
fwrite(comm_encoder, ['init' char(10)]);
pause(1);

%% start synchronizacji i wyczyszczenie bufora
fwrite(comm_encoder, ['sync' char(10)]);
pause(0.2);
dump = [];
while(comm_encoder.BytesAvailable > 0)
    dump = char(fread(comm_encoder, comm_encoder.BytesAvailable, 'char')');
    pause(0.1);
end;

pause(2);

for i = 1 : 100
    %%

    %  
    % Macierz sterujaca Q:
    % kolumna - numer generatora
    % wiersz 1 - start
    % wiersz 2 - stop
    % wartosci od 0 do 20000 (impulsy enkodera na obrót)

    Q = [1000 2000 3000 4000 5000 6000; ...
         3000 4000 5000 6000 7000 8000]*2;

    Q = [3000  6000  9000   12000 15000 18000; ...
         8000  11000 14000  17000 20000 3000];


    %% wyslanie punktów startu i stopu

    if (size(Q, 1) ~= 2 && size(Q, 2) ~= 6)
         error 'Niew³aœciwe wymiary macierzy steruj¹cej';
    end;
    Qint = int32(Q);

    data = sprintf('%04X', Qint(:));
    crc = uint16(0);
    for bajt = 1 : length(data)
        crc = bitxor(crc, uint16(data(bajt)));
        crc = bitshift(crc, 1);
%         C(i) = crc;
    end;

    msg = ['#' data sprintf('%04X', crc)];
    
    while(comm_encoder.BytesAvailable > 0)
        dump = char(fread(comm_encoder, comm_encoder.BytesAvailable, 'char')');
        pause(0.1);
    end;    


    fprintf('%s Wysylanie punktów startu...', datestr(now,'hh:MM:ss'));
    
    fwrite(comm_encoder, msg);
    pause(0.1);
    if comm_encoder.BytesAvailable ~= 1
        fprintf('B³¹d (1): %d bajtow\n', comm_encoder.BytesAvailable);
    else
        resp = fread(comm_encoder, 1);
        if (resp ~= '@')
            fprintf('B³¹d (2): znak %d\n', resp);
        else
            fprintf('OK\n');
        end;
    end;

end; % for

%%

fclose(comm_encoder);
