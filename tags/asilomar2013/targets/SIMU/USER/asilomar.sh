#!/bin/bash
sudo -E ./oaisim -c41 -y1 -A AWGN         > phy_awgn.txt 2> phy_awgn.csv
sudo -E ./oaisim -c41 -y1 -A AWGN -a      > abs_awgn.txt 2> abs_awgn.csv
sudo -E ./oaisim -c41 -y1 -A Rayleigh8    > phy_r8.txt   2> phy_r8.csv
sudo -E ./oaisim -c41 -y1 -A Rayleigh8 -a > abs_r8.txt   2> abs_r8.csv
sudo -E ./oaisim -c41 -y1 -A SCM_C        > phy_scmc.txt 2> phy_scmc.csv
sudo -E ./oaisim -c41 -y1 -A SCM_C -a     > abs_scmc.txt 2> abs_scmc.csv
