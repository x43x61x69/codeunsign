//
//  codeunsign.c
//  codeunsign
//
//  Copyright (c) 2014 Cai, Zhi-Wei. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------
//
// To compile: clang codeunsign.c -O2 -o codeunsign
//

#define VERSION "0.1"

#include <fcntl.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    
    printf("\n"
           "\n\t"
           "     _/_/_/                  _/                         \n\t"
           "  _/          _/_/      _/_/_/    _/_/    Copyright     \n\t"
           " _/        _/    _/  _/    _/  _/_/_/_/    © 2014       \n\t"
           "_/        _/    _/  _/    _/  _/         Cai, Zhi-Wei   \n\t"
           " _/_/_/    _/_/      _/_/_/    _/_/_/       v %s       \n\t"
           "   _/    _/                      _/                     \n\t"
           "  _/    _/  _/_/_/      _/_/_/        _/_/_/  _/_/_/    \n\t"
           " _/    _/  _/    _/  _/_/      _/  _/    _/  _/    _/   \n\t"
           "_/    _/  _/    _/      _/_/  _/  _/    _/  _/    _/    \n\t"
           " _/_/    _/    _/  _/_/_/    _/    _/_/_/  _/    _/     \n\t"
           "                                      _/                \n\t"
           "    - http://reversi.ng/ -        _/_/  \n\n\n", VERSION);
    
    if (argc < 2) {
        
        printf("usage: codeunsign target_file\n");
        
        return 0;
    }
    
    char                    *addr       = NULL;
    int                     fd          = open(argv[1], O_RDWR);
    size_t                  size;
    struct stat             stat_buf;
    struct fat_arch         *fa;
    struct fat_header       *fh;
    struct mach_header      *mh;
    struct mach_header_64   *mh64;
    struct load_command     *lc;
    uint32_t                mm;
    uint32_t                err;
    
    fstat(fd, &stat_buf);
    size                  = stat_buf.st_size;
    addr                  = mmap(0, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
    mm                    = *(uint32_t *)(addr);
    
    printf("* MH_MAGIC:      %04X\n", mm);
    
    switch(mm) {
        case MH_MAGIC:
            mh                    = (struct mach_header *)addr;
            addr                  += sizeof(struct mach_header);
            
            printf("* Architecture:  i386\n");
            printf("* Load Commands: %d\n", mh->ncmds);
            
            err                   = 2;
            for (int i=0; i<mh->ncmds; i++) {
                
                lc = (struct load_command *)addr;
                
                if (lc->cmd == LC_CODE_SIGNATURE) {
                    // LC_CODE_SIGNATURE found
                    printf("* Patching...\n");
                    err                   = 0;
                    mh->ncmds             -= 1;
                    mh->sizeofcmds        -= lc->cmdsize;
                    lc->cmd               = 0;
                    lc->cmdsize           = 0;
                    
                }
                // Next load command
                addr                  += lc->cmdsize;
            }
            msync(addr, size, MS_ASYNC);
            break;
        case MH_MAGIC_64:
            mh64                  = (struct mach_header_64 *)addr;
            addr                  += sizeof(struct mach_header_64);
            
            printf("* Architecture:  x86_64\n");
            printf("* Load Commands: %d\n", mh64->ncmds);
            
            err                   = 2;
            for (int i            = 0; i < mh64->ncmds; i++) {
                lc                    = (struct load_command *)addr;
                
                if (lc->cmd == LC_CODE_SIGNATURE) {
                    
                    printf("* Patching...\n");
                    err                   = 0;
                    mh64->ncmds           -= 1;
                    mh64->sizeofcmds      -= lc->cmdsize;
                    lc->cmd               = 0;
                    lc->cmdsize           = 0;
                }
                addr                  += lc->cmdsize;
            }
            msync(addr, size, MS_ASYNC);
            break;
        case FAT_CIGAM:
            fh                    = (struct fat_header *)addr;
            uint32_t i            = 0;
            uint32_t nfat_arch    = OSSwapBigToHostInt32(fh->nfat_arch); // In fat_arch struct the attributes are stored in big-endian.
            
            printf("* Mach-O Type: Fat\n");
            printf("* Architectures: %x\n", nfat_arch);
            
            fa                    = (struct fat_arch *)(addr + sizeof(struct fat_header));
            for(;nfat_arch-- > 0; fa++) {
                
                uint32_t offset, cputype;
                cputype               = OSSwapBigToHostInt32(fa->cputype);
                offset                = OSSwapBigToHostInt32(fa->offset);
                char *addrTemp        = NULL;
                
                switch(cputype) {
                    case 0x7: // 32bit
                        
                        addrTemp              = mmap(0, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
                        mh                    = (struct mach_header *)(addrTemp + offset);
                        addrTemp              += sizeof(struct mach_header) + offset;
                        
                        printf("* MH_MAGIC:      %04X\n\t", mh->magic);
                        printf("* Load Commands: %d\n\t", mh->ncmds);
                        
                        err                   = 3;
                        for (int i            = 0; i < mh->ncmds; i++) {
                            lc                    = (struct load_command *)addrTemp;
                            
                            if (lc->cmd == LC_CODE_SIGNATURE) {
                                
                                printf("* Patching...\n");
                                err                   = 0;
                                mh->ncmds             -= 1;
                                mh->sizeofcmds        -= lc->cmdsize;
                                lc->cmd               = 0;
                                lc->cmdsize           = 0;
                            }
                            addrTemp              += lc->cmdsize;
                        }
                        break;
                    case 0x1000007: // 64bit
                        
                        addrTemp              = mmap(0, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
                        mh64                  = (struct mach_header_64 *)(addrTemp + offset);
                        addrTemp              += sizeof(struct mach_header_64) + offset;
                        
                        printf("* MH_MAGIC_64:   %04X\n\t", mh64->magic);
                        printf("* Load Commands: %d\n\t", mh64->ncmds);
                        
                        err                   = 3;
                        for (int i            = 0; i < mh64->ncmds; i++) {
                            lc                    = (struct load_command *)addrTemp;
                            
                            if (lc->cmd == LC_CODE_SIGNATURE) {
                                
                                printf("* Patching...\n");
                                err                   = 0;
                                mh64->ncmds           -= 1;
                                mh64->sizeofcmds      -= lc->cmdsize;
                                lc->cmd               = 0;
                                lc->cmdsize           = 0;
                            }
                            addrTemp              += lc->cmdsize;
                        }
                        break;
                }
                if(err) printf("[ERROR] LC_CODE_SIGNATURE NOT FOUND.\n");
                msync(addrTemp, size, MS_ASYNC);
                munmap(addrTemp, size);
            }
            break;
        default:
            printf("[ERROR] INVALID MACH-O BINARY.\n");
            err                   = 1;
    }
    printf("\n");
    switch(err) {
        case 1:
            printf("[ERROR] ABORT.\n");
            break;
        case 2:
            printf("[ERROR] LC_CODE_SIGNATURE NOT FOUND.\n");
            break;
    }
    munmap(addr, size);
    close(fd);
    
    printf("\n* All done. :)\n\n");
    
    return 0;
}