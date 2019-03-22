
clear;

fprintf('on: %s\n', Sync_CreateMessage(1, 'on'));
fprintf('off: %s\n', Sync_CreateMessage(1, 'off'));

fprintf('p: %s\n', Sync_CreateMessage(6, [1000 5000 9000 16000]));