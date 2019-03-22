function str = Word2String( val )
    val = uint16(val);
    q3 = char('A'+bitand(bitshift(val, -12), 15));
    q2 = char('A'+bitand(bitshift(val, -8), 15));
    q1 = char('A'+bitand(bitshift(val, -4), 15));
    q0 = char('A'+bitand(bitshift(val, 0), 15));
    str = [q3 q2 q1 q0];
end

