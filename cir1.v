module top(a0, a1, b1, b0, c, h0, h1, m0, m1);
input a0, a1, b1, b0, c;
output h0, h1, m0, m1;
wire na, a, b, ne, e, d, na0, nb1, nb0;

and ( h0, a0, b0 );
xor ( h1, na, b );
not ( m0, d );
xor ( m1, b0, c );
and ( a, a1, b0 );
not ( na, a);
not ( na0, a0 );
not ( nb1, b1 );
or ( b, na0, nb1 );
and ( e, b0, c);
not ( ne, e );
xor ( d, ne, b1);
endmodule

