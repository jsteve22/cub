def bmm(x, y, q):      
  mask = (1<<32) - 1
  x_low = (x&mask)
  x_high = (x>>16)
  x_high = (x_high>>16) & mask
  assert x_high == (x>>32) 

  y_low = (y&mask)
  y_high = (y>>16)
  y_high = (y_high>>16) & mask
  assert y_high == (y>>32)


  z_low = (x_low * y_low)
  z_mid = (x_low * y_high) + (x_high * y_low) 
  z_high = (x_high * y_high) 
  if (z_low > 2**64 - 1):
    print('overflow 1')
  if (z_mid > 2**63):
    print('overflow 2')
  if (z_high > 2**63):
    print('overflow 3')

  assert (z_low + (z_mid<<32) + (z_high<<64) ) == x*y  

  shifting = 16

  for _ in range(32//shifting):
    z_mid = (z_mid<<shifting)%q
    if (z_mid > 2**63):
      print('overflow')
    z_high = (z_high<<shifting)%q
    if (z_high > 2**63):
      print('overflow')
    z_high = (z_high<<shifting)%q
    if (z_high > 2**63):
      print('overflow')

  ret = (z_low%q) + (z_mid%q) + (z_high%q)
  if (ret > 2**63):
      print('overflow')
  return (ret%q)

def main():
  x = 141600765125
  y = 260869796875
  q = 274877908993
  log_2 = 1
  log = 0
  while log_2 < q:
    log_2 = log_2 << 1
    log += 1
  import numpy as np
  print( np.log2(log_2%q))
  print( np.log2(pow(2**20, -1, q)) )
  print(q)

  print( bmm(x, y, q) )
  print( (x*y)%q )
  return

if __name__ == '__main__':
  main()