#include "common.h"
#include "iop.h"

struct scph1001
{
    u8* bios; //[0x80000]
    u8* main_ram; //[0x200000]

    iop_cpu* iop;

    FILE* reg_access_log;

    void init();
    void exit();
};

void scph1001::init()
{
    bios = (u8*)calloc(0x80000, 1);
    main_ram = (u8*)calloc(0x200000, 1);

    reg_access_log = fopen("reglog.txt","w+");
}

void scph1001::exit()
{
    free(bios);
    free(main_ram);

    if(reg_access_log) fclose(reg_access_log);
}

u8 scph1001_rb(void* dev, u32 addr)
{
    scph1001* device = (scph1001*)dev;
    if(addr <= 0x001fffff)
    {
        return device->main_ram[addr & 0x1fffff];
    }
    else if(addr >= 0x1f801000 && addr <= 0x1f802000)
    {
        fprintf(device->reg_access_log, "[CPU] Unknown address %08x\n", addr);
    }
    else if(addr >= 0x1fc00000 && addr <= 0x1fc7ffff)
    {
        return device->bios[addr & 0x7ffff];
    }
    return 0;
}

u16 scph1001_rh(void* dev, u32 addr)
{
    scph1001* device = (scph1001*)dev;
    if(addr <= 0x001fffff)
    {
        return *(u16*)(device->main_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1f801000 && addr <= 0x1f802000)
    {
        fprintf(device->reg_access_log, "[CPU] Unknown address %08x\n", addr);
    }
    else if(addr >= 0x1fc00000 && addr <= 0x1fc7ffff)
    {
        return *(u16*)(device->bios + (addr & 0x7ffff));
    }
    return 0;
}

u32 scph1001_rw(void* dev, u32 addr)
{
    scph1001* device = (scph1001*)dev;
    if(addr <= 0x001fffff)
    {
        return *(u32*)(device->main_ram + (addr & 0x1fffff));
    }
    else if(addr >= 0x1f801000 && addr <= 0x1f802000)
    {
        if(addr == 0x1f801814) return 0x1c000000;
        fprintf(device->reg_access_log, "[CPU] Unknown address %08x\n", addr);
    }
    else if(addr >= 0x1fc00000 && addr <= 0x1fc7ffff)
    {
        return *(u32*)(device->bios + (addr & 0x7ffff));
    }
    return 0;
}

void scph1001_wb(void* dev, u32 addr, u8 data)
{
    scph1001* device = (scph1001*)dev;
    if(addr < 0x001fffff)
    {
        device->main_ram[addr & 0x1fffff] = data;
    }
    else if(addr >= 0x1f801000 && addr <= 0x1f802000)
    {
        fprintf(device->reg_access_log, "[CPU] Unknown address %08x data %02x\n", addr, data);
    }
}

void scph1001_wh(void* dev, u32 addr, u16 data)
{
    scph1001* device = (scph1001*)dev;
    if(addr < 0x001fffff)
    {
        *(u16*)(device->main_ram + (addr & 0x1fffff)) = data;
    }
    else if(addr >= 0x1f801000 && addr <= 0x1f802000)
    {
        /*if(!(addr >= 0x1f801c00 && addr <= 0x1f801dff))*/ fprintf(device->reg_access_log, "[CPU] Unknown address %08x data %04x\n", addr, data);
    }
}

void scph1001_ww(void* dev, u32 addr, u32 data)
{
    scph1001* device = (scph1001*)dev;
    if(addr < 0x001fffff)
    {
        *(u32*)(device->main_ram + (addr & 0x1fffff)) = data;
    }
    else if(addr >= 0x1f801000 && addr <= 0x1f802000)
    {
        fprintf(device->reg_access_log, "[CPU] Unknown address %08x data %08x\n", addr, data);
    }
}

int main(int ac, char** av)
{
    if(ac < 2)
    {
        printf("Usage: %s [bios_file]\n", av[0]);
        return 1;
    }
    scph1001 dev;
    iop_cpu iop;

    iop.init();

    dev.iop = &iop;

    dev.init();

    iop.device = &dev;

    iop.rb_real = scph1001_rb;
    iop.rh_real = scph1001_rh;
    iop.rw_real = scph1001_rw;
    
    iop.wb_real = scph1001_wb;
    iop.wh_real = scph1001_wh;
    iop.ww_real = scph1001_ww;

    FILE* fp = fopen(av[1], "rb");
    if(!fp)
    {
        printf("unable to open %s, are you sure it exists?\n", av[1]);
        return 3;
    }
    if(fread(dev.bios, 1, 0x80000, fp) != 0x80000)
    {
        fclose(fp);
        return 4;
    }
    fclose(fp);

    for(int i = 0; i < 35000000; i++)
    {
        iop.tick();
    }

    iop.exit();
    dev.exit();
}