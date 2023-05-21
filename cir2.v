module top(x0, x1, y1, y0, z, u0, u1, w0, w1);
input x0, x1, y1, y0, z;
output u0, u1, w0, w1;
wire na, a, b, ne, e, d, na0, nb1, nb;

and ( u0, x0, y0 );
xor ( u1, na, nb );
not ( w0, d );
xor ( w1, y0, z );
and ( a, x1, y0 );
not ( na, a);
and ( b, y1, x0 );
not ( nb, b );
and ( e, y0, z);
not ( ne, e );
xor ( d, ne, y1);
endmodule

