
program
var k, f : int;
    notUsed : int; // Unusable variable warning example
begin
   // Single line comment example
begin
   // Factorial calculation
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
