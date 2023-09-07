from poly import Poly
from bb import bmm

def main():
  size = 256
  q = 3329
  psi = 17


  '''
  size = 2**11
  q = 274_877_908_993
  psi = 124878001
  '''

  n = 2
  size *= 2
  n *= 2

  invpsi = pow(psi, -1, q)

  fn = Poly([1] + ([0]*(size-1)) + [1])  

  a = Poly([1, 2, 3, 4])
  a = Poly(list(range(size)))
  b = Poly([0, 2, 3, 4])

  z = a*b
  quo, rem = z // fn

  psi_powers = []
  invpsi_powers = []
  for i in range(size//n):
    psi_powers.append( pow(psi, bitrev(i, size//n), q) )
    invpsi_powers.append( pow(invpsi, bitrev(i, size//n), q) )
  
  psi_poly = []
  powers = []
  for i in range(size//n):
    powers.append( pow(psi, 2*bitrev(i, size//n) + 1, q) )
    p = Poly([-1*(pow(psi, 2*bitrev(i, size//n) + 1, q)), 0, 1])
    p = p % q
    psi_poly.append(p)
  prod = Poly([1])
  for ind, p in enumerate(psi_poly):
    prod = prod * p
    prod = prod % q

  na = NTT(a, size, q, psi_powers, n)
  na = na % q
  ia = iNTT(na, size, q, psi_powers, n)
  ia = ia % q
  # print(ia)
  
  nb = NTT(b, size, q, psi_powers, n)
  nb = nb % q
  ib = iNTT(nb, size, q, psi_powers, n)
  # print(ib)

  # c = [ (xa * xb) % q for xa, xb in zip(na, nb) ]
  # print(powers)
  c = ntt_mult(na, nb, size, q, powers, n)
  ic = iNTT( c, size, q, psi_powers, n )
  rem = rem % q
  print(rem)
  print()
  print(ic)
  print()
  print( ic == rem )
  return

  print( powers[:64] )
  test = Poly( list(range(size)) )
  zest = NTT(test, size, q, psi_powers)
  zest = zest % q
  mest = ntt_mult(zest, zest, size, q, powers)
  # print(mest.poly[:64])
  best = iNTT( mest, size, q, psi_powers)
  z = test * test
  quo, rem = z // fn
  rem = rem % q
  print(best.poly[:10] )
  print( rem == best )

  return

def ntt_mult(poly_a, poly_b, size, q, psi, nsz=2):
  a = poly_a.poly.copy()
  b = poly_b.poly.copy()

  c = []

  for i in range(0, size//nsz):
    di = i*nsz
    # psi_p = pow(psi, 2*bitrev(i, size//2)+1, q)
    psi_p = psi[i]
    c += __testmul(a[di:di+nsz], b[di:di+nsz], q, psi_p, nsz)
  return Poly(c)

def basemul(poly_a, poly_b, q, psi):
  r = [0, 0]
  r[0]  = (poly_a[1] * poly_b[1]) % q
  r[0]  = (r[0] * psi) % q
  r[0] += (poly_a[0] * poly_b[0])
  r[0] %= q
  r[1]  = (poly_a[0] * poly_b[1]) % q
  r[1] += (poly_a[1] * poly_b[0]) % q
  r[1] %= q
  return r

def testmul(poly_a, poly_b, q, psi, size=2):
  r = [0 for _ in range(size)]
  for i in range(size):
    for j in range(size):
      r[(i+j)%size] += poly_a[i] * poly_b[j] if i+j < size else ( poly_a[i] * poly_b[j] * psi )
  r = [ri % q for ri in r]
  return r

def __testmul(poly_a, poly_b, q, psi, size=2):
  r = [0 for _ in range(size)]
  for i in range(1, size):
    for j in range(size - i, size):
      r[i+j-size] += (poly_a[i]*poly_b[j]*psi)
  for i in range(size):
    for j in range(size-i):
      r[i+j] += (poly_a[i] * poly_b[j])
  r = [ri % q for ri in r]
  return r

def NTT(poly, size, mod, psi_powers, n=2):
  r = poly.copy()
  k = 1
  length = size // 2
  # length starts at half, and goes to n where (x^n - psi)
  while length >= n:
    start = 0
    while start < size:
      zeta = psi_powers[k]
      k += 1
      for j in range(start, start+length):
        # t = (zeta * r[j + length]) % mod
        t = bmm(zeta, r[j + length], mod)
        r[j + length] = r[j] - t
        r[j] = r[j] + t
        r[j + length] %= mod
        r[j] %= mod
      start = start + length + length
    length = length >> 1
  return r
  '''
  ret = [0]*size
  f = poly.copy()
  half = size//2
  for i in range(half):
    s0 = 0
    s1 = 0
    for j in range(half):
      index = (2*bitrev(i, half) + 1)*j
      index %= size
      s0 += (f[2*j])*psi_powers[index]
      s1 += (f[2*j+1])*psi_powers[index]
    ret[2*i] = s0
    ret[2*i + 1] = s1
  return Poly(ret)   
  '''

def iNTT(poly, size, mod, psi_powers, n=2):
  r = poly.copy()

  k = (size//n) - 1
  length = n
  # length starts at n where (x^n - psi)
  # length ends at half
  while length <= (size//2):
    start = 0
    while start < size:
      zeta = psi_powers[k]
      k -= 1
      for j in range(start, start + length):
        t = r[j]
        r[j] = (t + r[j + length]) % mod
        r[j + length] = r[j + length] - t
        r[j + length] = (zeta*r[j + length]) % mod
      start = start + length + length
    length = length << 1

  invsize = pow(size//n, -1, mod)
  # invsize = 1175
  # invsize = 1441
  for i in range(size):
    r[i] = (r[i]*invsize) % mod
    pass
  return r   

def bitrev(k: int, size: int):
  # return bit-reversed order of k
  
  revk = 0
  bit_len = (size-1).bit_length()

  for i in range( bit_len ):
    revk += (2 ** (bit_len-1-i) ) * ( ((2**i) & k ) >> i )

  return revk

def __NTT(p: Poly, size, mod, psi):
  # this will convert p into NTT

  a = p.copy()
  # ret = Poly( ret.poly + ([0]*(self.n-len(p))) )

  m = 1
  k = size // 2
  while m < size:
    for i in range(m):
      jFirst = 2 * i * k
      jLast = jFirst + k
      wi = psi[ m+i ]
      for j in range(jFirst,jLast):
        l = j + k
        t = a[j]
        u = a[l] * wi
        a[j] = (t + u) % mod
        a[l] = (t - u) % mod

    m = m * 2
    k = k//2

  return a 


if __name__ == '__main__':
  main()