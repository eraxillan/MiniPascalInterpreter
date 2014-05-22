
program
var k, f : int;
    notUsed : int; // пример предупреждения о неиспользуемой переменной
begin
   // Однострочный комментарий
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
