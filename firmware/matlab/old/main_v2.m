close all;
clear; clc;
%%

% wylacz wszystkie
Sync_Send(Sync_CreateMessage(1, 'off'));
Sync_Send(Sync_CreateMessage(2, 'off'));
Sync_Send(Sync_CreateMessage(3, 'off'));
Sync_Send(Sync_CreateMessage(4, 'off'));
Sync_Send(Sync_CreateMessage(5, 'off'));
Sync_Send(Sync_CreateMessage(6, 'off'));

% wlacz wszystkie
if 0
%     Sync_Send(Sync_CreateMessage(2, 'on'));
    Sync_Send(Sync_CreateMessage(4, 'on'));
    Sync_Send(Sync_CreateMessage(5, 'on'));
    
    %%
    start = 5000;
    stop = 8500;
    Q = [start stop]+12000;
%     Sync_Send(Sync_CreateMessage(2, Q));
    Sync_Send(Sync_CreateMessage(4, Q));
    Sync_Send(Sync_CreateMessage(5, Q));
    %%
    
end;

Sync_Send(Sync_CreateMessage(1, [0 11000]));
Sync_Send(Sync_CreateMessage(2, [0 11000]));
Sync_Send(Sync_CreateMessage(3, [0 11000]));
Sync_Send(Sync_CreateMessage(4, [0 11000]));
Sync_Send(Sync_CreateMessage(5, [0 11000]));
Sync_Send(Sync_CreateMessage(6, [0 11000]));


Sync_Done();



% 
% for i = 1 : 100
%     %%
% 
%     %  
%     % Macierz sterujaca Q:
%     % kolumna - numer generatora
%     % wiersz 1 - start
%     % wiersz 2 - stop
%     % wartosci od 0 do 20000 (impulsy enkodera na obrót)
% 
%     Q = [1000 5000 9000 13000];
%     
% %     Q = [1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500];
%     CH = 1;
% 
% 
%     %% wyslanie punktów startu i stopu
% 
%     Q = Q(:)';
%     if (length(Q) > 16 * 2 || mod(uint32(length(Q)), 2) ~= 0)
%          error 'Niew³aœciwe wymiary macierzy steruj¹cej';
%     end;
%     if (CH < 1 || CH > 6)
%          error 'Niew³aœciwe numer  kana³u';
%     end;        
% 
%     % konwersja na tekst
%     pairs = length(Q) / 2;
%     msg = [char('A'+CH) char('A'+pairs)];
%     for val = Q
%         msg = [msg Word2String(val)];
%     end;
%     
%     crc = uint16(0);
%     for bajt = 1 : length(msg)
%         crc = bitxor(crc, uint16(msg(bajt)));
%         crc = bitshift(crc, 1);
%     end;
% 
%     msg = ['#' msg Word2String(crc) '%'];
%     
%     while(comm_encoder.BytesAvailable > 0)
%         dump = char(fread(comm_encoder, comm_encoder.BytesAvailable, 'char')');
%         pause(0.1);
%     end;    
% 
% 
%     fprintf('%s Wysylanie punktów startu dla kana³u %d...', datestr(now,'hh:MM:ss'), CH);
%     
%     fwrite(comm_encoder, msg);
%     pause(0.1);
%     if comm_encoder.BytesAvailable ~= 1
%         fprintf('B³¹d (1): %d bajtow\n', comm_encoder.BytesAvailable);
%     else
%         resp = fread(comm_encoder, 1);
%         if (resp ~= '@')
%             fprintf('B³¹d (2): znak %d\n', resp);
%         else
%             fprintf('OK\n');
%         end;
%     end;
% 
% end; % for
% 
% %%
% 
% fclose(comm_encoder);
