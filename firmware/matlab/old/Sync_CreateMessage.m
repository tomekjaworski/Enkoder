function msg = Sync_CreateMessage(channel, points)

    c = class(points);
    if (strcmp(c, 'char'))
        if (strcmp(upper(points), 'ON'))
            Q = [hex2dec('FFFF') hex2dec('FFFF')];
        elseif (strcmp(upper(points), 'OFF'))
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

