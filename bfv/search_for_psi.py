def main():
  q = 274_877_908_993
  psi = 124878001

  start = 29373000000
  start = 64497000000
  q    = 274877908993

  print(f'q: {q}')
  print(f'psi: {psi}')
  print(f'psi^(2^11) % q: {pow(psi, 2**11, q)}')

  for i in range(start, q):
    if pow(i, 2, q) == psi:
      print(f'NUMBER: {i}')
      break
    if (i % 1_000_000 == 0):
      print(i)

if __name__ == '__main__':
  main()