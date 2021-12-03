import numpy as np

lsb = np.arange(0, 128)
file1 = open('test_data_gen_1.txt', 'w')

file1.write('{')
for i in range(5):
    file1.write('{')
    for j in range(6):
        for k in lsb:
            file1.write(str(i * 1000 + j * 128 + k) + ',')

    file1.write('},\n')
file1.write('}')
file1.close()

#file1 = open('test_data_gen_2.txt', 'w')