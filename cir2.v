module top( s, t, u, v, w, x, y, z );
input s, t, u, v, w;
output x, y, z;
wire tn;
not ( tn, t );
and ( x, w, v);
or ( y, w, u );
xor ( z, y, x );
endmodule