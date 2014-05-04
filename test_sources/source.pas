
program
var k1, k, f : int;
    b1, b : bool;
    notUsed : int; // warning example.
begin
   // Line comment.
begin
   // Factorial
   read (k);
   if k = 0 then k := 1;
   f := 1;
   do
   begin
      f := f * k;
      k := k - 1;
   end;
   while k > 1;
   write (f);
end
end.