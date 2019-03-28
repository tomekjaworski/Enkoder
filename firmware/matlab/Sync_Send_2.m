function Sync_Send(command, arg)
%Sync_Send_2('init') - inicjalizacja ukÅ‚adu
%Sync_Send_2('done') - wyÅ‚Ä…czenie ukÅ‚adu
%Sync_Send_2(nr_kanalu(1-6), pozycje katowe wÅ‚Ä…cz-wyÅ‚Ä…cz) - sterowanie blokadÄ… generatorÃ³w
%Sync_Send_2(10,[a b])  - sterowanie natryskiwaniem, piÄ™Ä‡ ostatnich bitÃ³w zmiennej a odpowiada zadanym stanom natryskiwaczy
%Sync_Send_2(11, [0 0]) - odczyt licznika liczby obrotÃ³w taÅ›my
    global sync_obj;
    
    if ischar(command)
        if strcmpi(command, 'done')
            Sync_Done();
            return;
        end;
        
        if strcmpi(command, 'init')
            if (~isempty(sync_obj))  
                Sync_Done();
            end;
            Sync_Init();
            return;
        end;
        
        error('SYNC: Nieznane polecenie ''%s''\n', command);
    end;
    
    if (isa(command, 'double'))
        assert(nargin == 2, 'SYNC: Niew³aœciwa iloœæ parametrów');
        msg = Sync_CreateMessage(command, arg);
    end;

            

    fprintf('SYNC%d: Wysylanie ''%s''... ', msg(2) - 'A', msg);
    fwrite(sync_obj.conn, msg);
    pause(0.1);
    if sync_obj.conn.BytesAvailable > 1
        resp = fread(sync_obj.conn, sync_obj.conn.BytesAvailable);
        fprintf('B³¹d (1): %d bajtow (%s)\n', length(resp), resp);
    else
        resp = fread(sync_obj.conn, 1);
        if (resp ~= '@')
            fprintf('B³¹d (2): znak %d (%s)\n', resp, resp);
        else
            fprintf('OK\n');
        end;
    end;

end

function  Sync_Done()

    global sync_obj;
    if (isempty(sync_obj))
        return;
    end;
    fprintf('SYNC: Zamykanie polaczenia...\n');
    fclose(sync_obj.conn);
    pause(0.2);
    sync_obj = [];

end

function Sync_Init()
	global sync_obj;
    if (isempty(sync_obj))
        
        comm_encoder = tcpip('192.168.0.19', 20000);
        comm_encoder.Timeout = 1;
        comm_encoder.InputBufferSize = 10 * 1024;

        fprintf('SYNC: Laczenie z enkoderem...\n');
        fopen(comm_encoder);
        sync_obj.conn = comm_encoder;

        fprintf('SYNC: Uruchamianie kontrolera enkodera... ');
        fwrite(sync_obj.conn, ['+++' char(10)]);
        pause(0.2);
        fwrite(sync_obj.conn, ['init' char(10)]);
        pause(1);

        %% start synchronizacji i wyczyszczenie bufora
        fwrite(sync_obj.conn, ['sync' char(10)]);
        pause(0.2);
        dump = [];
        while(sync_obj.conn.BytesAvailable > 0)
            dump = char(fread(sync_obj.conn, sync_obj.conn.BytesAvailable, 'char')');
            pause(0.1);
        end;

        pause(2);     
        
        fprintf('Ok (%d)\n', length(dump));
        
        
    end;
end

function msg = Sync_CreateMessage(channel, points)

    c = class(points);
    if (ischar(points))
        if (strcmpi(points, 'ON'))
            Q = [hex2dec('FFFF') hex2dec('FFFF')];
        elseif (strcmpi(points, 'OFF'))
            Q = [hex2dec('FF00') hex2dec('FF00')];
        else
            error(['Nieznane polecenie ' points]);
        end;
    else
        %double
        assert(strcmp(c, 'double'));
        Q = points(:)';
    end;
        
    CH = channel(:);

    if (length(Q) > 16 * 2 || mod(uint32(length(Q)), 2) ~= 0)
         error 'Niew³aœciwe wymiary macierzy steruj¹cej';
    end;
    if (CH < 1 || CH > 6)
         error 'Niew³aœciwe numer  kana³u';
    end;        

    % konwersja na tekst
    pairs = length(Q) / 2;
    msg = [char('A'+CH) char('A'+pairs)];
    for val = Q
        msg = [msg Word2String(val)];
    end;
    
    crc = uint16(0);
    for bajt = 1 : length(msg)
        crc = bitxor(crc, uint16(msg(bajt)));
        crc = bitshift(crc, 1);
    end;

    msg = ['#' msg Word2String(crc) '%'];

end

function str = Word2String( val )
    val = uint16(val);
    q3 = char('A'+bitand(bitshift(val, -12), 15));
    q2 = char('A'+bitand(bitshift(val, -8), 15));
    q1 = char('A'+bitand(bitshift(val, -4), 15));
    q0 = char('A'+bitand(bitshift(val, 0), 15));
    str = [q3 q2 q1 q0];
end




