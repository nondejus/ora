/* Simulator for the moxie processor
   Copyright 2014 Anthony Green
   Distributed under the MIT/X11 software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#define TRACE(str) if (tracing) fprintf(tracefile,"0x%08x, %s, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", opc, str, cpu.asregs.regs[0], cpu.asregs.regs[1], cpu.asregs.regs[2], cpu.asregs.regs[3], cpu.asregs.regs[4], cpu.asregs.regs[5], cpu.asregs.regs[6], cpu.asregs.regs[7], cpu.asregs.regs[8], cpu.asregs.regs[9], cpu.asregs.regs[10], cpu.asregs.regs[11], cpu.asregs.regs[12], cpu.asregs.regs[13], cpu.asregs.regs[14], cpu.asregs.regs[15]);

static int tracing = 0;

void
sim_resume (sd, step, siggnal)
     SIM_DESC sd;
     int step, siggnal;
{
  word pc, opc;
  unsigned long long insts;
  unsigned short inst;
  sim_cpu *scpu = STATE_CPU (sd, 0); /* FIXME */
  address_word cia = CIA_GET (scpu);

  cpu.asregs.exception = step ? SIGTRAP: 0;
  pc = cpu.asregs.regs[PC_REGNO];
  insts = cpu.asregs.insts;

  /* Run instructions here. */
  do 
    {
      opc = pc;

      /* Fetch the instruction at pc.  */
      inst = (sim_core_read_aligned_1 (scpu, cia, read_map, pc) << 8)
	+ sim_core_read_aligned_1 (scpu, cia, read_map, pc+1);

      /* Decode instruction.  */
      if (inst & (1 << 15))
	{
	  if (inst & (1 << 14))
	    {
	      /* This is a Form 3 instruction.  */
	      int opcode = (inst >> 10 & 0xf);

	      switch (opcode)
		{
		case 0x00: /* beq */
		  {
		    TRACE("beq");
		    if (cpu.asregs.cc & CC_EQ)
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x01: /* bne */
		  {
		    TRACE("bne");
		    if (! (cpu.asregs.cc & CC_EQ))
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x02: /* blt */
		  {
		    TRACE("blt");
		    if (cpu.asregs.cc & CC_LT)
		      pc += INST2OFFSET(inst);
		  }		  break;
		case 0x03: /* bgt */
		  {
		    TRACE("bgt");
		    if (cpu.asregs.cc & CC_GT)
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x04: /* bltu */
		  {
		    TRACE("bltu");
		    if (cpu.asregs.cc & CC_LTU)
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x05: /* bgtu */
		  {
		    TRACE("bgtu");
		    if (cpu.asregs.cc & CC_GTU)
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x06: /* bge */
		  {
		    TRACE("bge");
		    if (cpu.asregs.cc & (CC_GT | CC_EQ))
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x07: /* ble */
		  {
		    TRACE("ble");
		    if (cpu.asregs.cc & (CC_LT | CC_EQ))
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x08: /* bgeu */
		  {
		    TRACE("bgeu");
		    if (cpu.asregs.cc & (CC_GTU | CC_EQ))
		      pc += INST2OFFSET(inst);
		  }
		  break;
		case 0x09: /* bleu */
		  {
		    TRACE("bleu");
		    if (cpu.asregs.cc & (CC_LTU | CC_EQ))
		      pc += INST2OFFSET(inst);
		  }
		  break;
		default:
		  {
		    TRACE("SIGILL3");
		    cpu.asregs.exception = SIGILL;
		    break;
		  }
		}
	    }
	  else
	    {
	      /* This is a Form 2 instruction.  */
	      int opcode = (inst >> 12 & 0x3);
	      switch (opcode)
		{
		case 0x00: /* inc */
		  {
		    int a = (inst >> 8) & 0xf;
		    unsigned av = cpu.asregs.regs[a];
		    unsigned v = (inst & 0xff);
		    TRACE("inc");
		    cpu.asregs.regs[a] = av + v;
		  }
		  break;
		case 0x01: /* dec */
		  {
		    int a = (inst >> 8) & 0xf;
		    unsigned av = cpu.asregs.regs[a];
		    unsigned v = (inst & 0xff);
		    TRACE("dec");
		    cpu.asregs.regs[a] = av - v;
		  }
		  break;
		case 0x02: /* gsr */
		  {
		    int a = (inst >> 8) & 0xf;
		    unsigned v = (inst & 0xff);
		    TRACE("gsr");
		    cpu.asregs.regs[a] = cpu.asregs.sregs[v];
		  }
		  break;
		case 0x03: /* ssr */
		  {
		    int a = (inst >> 8) & 0xf;
		    unsigned v = (inst & 0xff);
		    TRACE("ssr");
		    cpu.asregs.sregs[v] = cpu.asregs.regs[a];
		  }
		  break;
		default:
		  TRACE("SIGILL2");
		  cpu.asregs.exception = SIGILL;
		  break;
		}
	    }
	}
      else
	{
	  /* This is a Form 1 instruction.  */
	  int opcode = inst >> 8;
	  switch (opcode)
	    {
	    case 0x00: /* bad */
	      opc = opcode;
	      TRACE("SIGILL0");
	      cpu.asregs.exception = SIGILL;
	      break;
	    case 0x01: /* ldi.l (immediate) */
	      {
		int reg = (inst >> 4) & 0xf;
		TRACE("ldi.l");
		unsigned int val = EXTRACT_WORD(pc+2);
		cpu.asregs.regs[reg] = val;
		pc += 4;
	      }
	      break;
	    case 0x02: /* mov (register-to-register) */
	      {
		int dest  = (inst >> 4) & 0xf;
		int src = (inst ) & 0xf;
		TRACE("mov");
		cpu.asregs.regs[dest] = cpu.asregs.regs[src];
	      }
	      break;
 	    case 0x03: /* jsra */
 	      {
 		unsigned int fn = EXTRACT_WORD(pc+2);
 		unsigned int sp = cpu.asregs.regs[1];
		TRACE("jsra");
 		/* Save a slot for the static chain.  */
		sp -= 4;

 		/* Push the return address.  */
		sp -= 4;
 		wlat (scpu, opc, sp, pc + 6);
 		
 		/* Push the current frame pointer.  */
 		sp -= 4;
 		wlat (scpu, opc, sp, cpu.asregs.regs[0]);
 
 		/* Uncache the stack pointer and set the pc and $fp.  */
		cpu.asregs.regs[1] = sp;
		cpu.asregs.regs[0] = sp;
 		pc = fn - 2;
 	      }
 	      break;
 	    case 0x04: /* ret */
 	      {
 		unsigned int sp = cpu.asregs.regs[0];

		TRACE("ret");
 
 		/* Pop the frame pointer.  */
 		cpu.asregs.regs[0] = rlat (scpu, opc, sp);
 		sp += 4;
 		
 		/* Pop the return address.  */
 		pc = rlat (scpu, opc, sp) - 2;
 		sp += 4;

		/* Skip over the static chain slot.  */
		sp += 4;
 
 		/* Uncache the stack pointer.  */
 		cpu.asregs.regs[1] = sp;
  	      }
  	      break;
	    case 0x05: /* add.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		unsigned av = cpu.asregs.regs[a];
		unsigned bv = cpu.asregs.regs[b];
		TRACE("add.l");
		cpu.asregs.regs[a] = av + bv;
	      }
	      break;
	    case 0x06: /* push */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int sp = cpu.asregs.regs[a] - 4;
		TRACE("push");
		wlat (scpu, opc, sp, cpu.asregs.regs[b]);
		cpu.asregs.regs[a] = sp;
	      }
	      break;
	    case 0x07: /* pop */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int sp = cpu.asregs.regs[a];
		TRACE("pop");
		cpu.asregs.regs[b] = rlat (scpu, opc, sp);
		cpu.asregs.regs[a] = sp + 4;
	      }
	      break;
	    case 0x08: /* lda.l */
	      {
		int reg = (inst >> 4) & 0xf;
		unsigned int addr = EXTRACT_WORD(pc+2);
		TRACE("lda.l");
		cpu.asregs.regs[reg] = rlat (scpu, opc, addr);
		pc += 4;
	      }
	      break;
	    case 0x09: /* sta.l */
	      {
		int reg = (inst >> 4) & 0xf;
		unsigned int addr = EXTRACT_WORD(pc+2);
		TRACE("sta.l");
		wlat (scpu, opc, addr, cpu.asregs.regs[reg]);
		pc += 4;
	      }
	      break;
	    case 0x0a: /* ld.l (register indirect) */
	      {
		int src  = inst & 0xf;
		int dest = (inst >> 4) & 0xf;
		int xv;
		TRACE("ld.l");
		xv = cpu.asregs.regs[src];
		cpu.asregs.regs[dest] = rlat (scpu, opc, xv);
	      }
	      break;
	    case 0x0b: /* st.l */
	      {
		int dest = (inst >> 4) & 0xf;
		int val  = inst & 0xf;
		TRACE("st.l");
		wlat (scpu, opc, cpu.asregs.regs[dest], cpu.asregs.regs[val]);
	      }
	      break;
	    case 0x0c: /* ldo.l */
	      {
		unsigned int addr = EXTRACT_WORD(pc+2);
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		TRACE("ldo.l");
		addr += cpu.asregs.regs[b];
		cpu.asregs.regs[a] = rlat (scpu, opc, addr);
		pc += 4;
	      }
	      break;
	    case 0x0d: /* sto.l */
	      {
		unsigned int addr = EXTRACT_WORD(pc+2);
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		TRACE("sto.l");
		addr += cpu.asregs.regs[a];
		wlat (scpu, opc, addr, cpu.asregs.regs[b]);
		pc += 4;
	      }
	      break;
	    case 0x0e: /* cmp */
	      {
		int a  = (inst >> 4) & 0xf;
		int b  = inst & 0xf;
		int cc = 0;
		int va = cpu.asregs.regs[a];
		int vb = cpu.asregs.regs[b]; 

		TRACE("cmp");

		if (va == vb)
		  cc = CC_EQ;
		else
		  {
		    cc |= (va < vb ? CC_LT : 0);
		    cc |= (va > vb ? CC_GT : 0);
		    cc |= ((unsigned int) va < (unsigned int) vb ? CC_LTU : 0);
		    cc |= ((unsigned int) va > (unsigned int) vb ? CC_GTU : 0);
		  }

		cpu.asregs.cc = cc;
	      }
	      break;
	    case 0x0f: /* nop */
	      break;
	    case 0x10: /* sex.b */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		signed char bv = cpu.asregs.regs[b];
		TRACE("sex.b");
		cpu.asregs.regs[a] = (int) bv;
	      }
	      break;
	    case 0x11: /* sex.s */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		signed short bv = cpu.asregs.regs[b];
		TRACE("sex.s");
		cpu.asregs.regs[a] = (int) bv;
	      }
	      break;
	    case 0x12: /* bad */
	    case 0x13: /* bad */
	    case 0x14: /* bad */
	    case 0x15: /* bad */
	    case 0x16: /* bad */
	    case 0x17: /* bad */
	    case 0x18: /* bad */
	      {
		opc = opcode;
		TRACE("SIGILL0");
		cpu.asregs.exception = SIGILL;
		break;
	      }
	    case 0x19: /* jsr */
	      {
		unsigned int fn = cpu.asregs.regs[(inst >> 4) & 0xf];
		unsigned int sp = cpu.asregs.regs[1];

		TRACE("jsr");

 		/* Save a slot for the static chain.  */
		sp -= 4;

		/* Push the return address.  */
		sp -= 4;
		wlat (scpu, opc, sp, pc + 2);
		
		/* Push the current frame pointer.  */
		sp -= 4;
		wlat (scpu, opc, sp, cpu.asregs.regs[0]);

		/* Uncache the stack pointer and set the fp & pc.  */
		cpu.asregs.regs[1] = sp;
		cpu.asregs.regs[0] = sp;
		pc = fn - 2;
	      }
	      break;
	    case 0x1a: /* jmpa */
	      {
		unsigned int tgt = EXTRACT_WORD(pc+2);
		TRACE("jmpa");
		pc = tgt - 2;
	      }
	      break;
	    case 0x1b: /* ldi.b (immediate) */
	      {
		int reg = (inst >> 4) & 0xf;

		unsigned int val = EXTRACT_WORD(pc+2);
		TRACE("ldi.b");
		cpu.asregs.regs[reg] = val;
		pc += 4;
	      }
	      break;
	    case 0x1c: /* ld.b (register indirect) */
	      {
		int src  = inst & 0xf;
		int dest = (inst >> 4) & 0xf;
		int xv;
		TRACE("ld.b");
		xv = cpu.asregs.regs[src];
		cpu.asregs.regs[dest] = rbat (scpu, opc, xv);
	      }
	      break;
	    case 0x1d: /* lda.b */
	      {
		int reg = (inst >> 4) & 0xf;
		unsigned int addr = EXTRACT_WORD(pc+2);
		TRACE("lda.b");
		cpu.asregs.regs[reg] = rbat (scpu, opc, addr);
		pc += 4;
	      }
	      break;
	    case 0x1e: /* st.b */
	      {
		int dest = (inst >> 4) & 0xf;
		int val  = inst & 0xf;
		TRACE("st.b");
		wbat (scpu, opc, cpu.asregs.regs[dest], cpu.asregs.regs[val]);
	      }
	      break;
	    case 0x1f: /* sta.b */
	      {
		int reg = (inst >> 4) & 0xf;
		unsigned int addr = EXTRACT_WORD(pc+2);
		TRACE("sta.b");
		wbat (scpu, opc, addr, cpu.asregs.regs[reg]);
		pc += 4;
	      }
	      break;
	    case 0x20: /* ldi.s (immediate) */
	      {
		int reg = (inst >> 4) & 0xf;

		unsigned int val = EXTRACT_WORD(pc+2);
		TRACE("ldi.s");
		cpu.asregs.regs[reg] = val;
		pc += 4;
	      }
	      break;
	    case 0x21: /* ld.s (register indirect) */
	      {
		int src  = inst & 0xf;
		int dest = (inst >> 4) & 0xf;
		int xv;
		TRACE("ld.s");
		xv = cpu.asregs.regs[src];
		cpu.asregs.regs[dest] = rsat (scpu, opc, xv);
	      }
	      break;
	    case 0x22: /* lda.s */
	      {
		int reg = (inst >> 4) & 0xf;
		unsigned int addr = EXTRACT_WORD(pc+2);
		TRACE("lda.s");
		cpu.asregs.regs[reg] = rsat (scpu, opc, addr);
		pc += 4;
	      }
	      break;
	    case 0x23: /* st.s */
	      {
		int dest = (inst >> 4) & 0xf;
		int val  = inst & 0xf;
		TRACE("st.s");
		wsat (scpu, opc, cpu.asregs.regs[dest], cpu.asregs.regs[val]);
	      }
	      break;
	    case 0x24: /* sta.s */
	      {
		int reg = (inst >> 4) & 0xf;
		unsigned int addr = EXTRACT_WORD(pc+2);
		TRACE("sta.s");
		wsat (scpu, opc, addr, cpu.asregs.regs[reg]);
		pc += 4;
	      }
	      break;
	    case 0x25: /* jmp */
	      {
		int reg = (inst >> 4) & 0xf;
		TRACE("jmp");
		pc = cpu.asregs.regs[reg] - 2;
	      }
	      break;
	    case 0x26: /* and */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av, bv;
		TRACE("and");
		av = cpu.asregs.regs[a];
		bv = cpu.asregs.regs[b];
		cpu.asregs.regs[a] = av & bv;
	      }
	      break;
	    case 0x27: /* lshr */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av = cpu.asregs.regs[a];
		int bv = cpu.asregs.regs[b];
		TRACE("lshr");
		cpu.asregs.regs[a] = (unsigned) ((unsigned) av >> bv);
	      }
	      break;
	    case 0x28: /* ashl */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av = cpu.asregs.regs[a];
		int bv = cpu.asregs.regs[b];
		TRACE("ashl");
		cpu.asregs.regs[a] = av << bv;
	      }
	      break;
	    case 0x29: /* sub.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		unsigned av = cpu.asregs.regs[a];
		unsigned bv = cpu.asregs.regs[b];
		TRACE("sub.l");
		cpu.asregs.regs[a] = av - bv;
	      }
	      break;
	    case 0x2a: /* neg */
	      {
		int a  = (inst >> 4) & 0xf;
		int b  = inst & 0xf;
		int bv = cpu.asregs.regs[b];
		TRACE("neg");
		cpu.asregs.regs[a] = - bv;
	      }
	      break;
	    case 0x2b: /* or */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av, bv;
		TRACE("or");
		av = cpu.asregs.regs[a];
		bv = cpu.asregs.regs[b];
		cpu.asregs.regs[a] = av | bv;
	      }
	      break;
	    case 0x2c: /* not */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int bv = cpu.asregs.regs[b];
		TRACE("not");
		cpu.asregs.regs[a] = 0xffffffff ^ bv;
	      }
	      break;
	    case 0x2d: /* ashr */
	      {
		int a  = (inst >> 4) & 0xf;
		int b  = inst & 0xf;
		int av = cpu.asregs.regs[a];
		int bv = cpu.asregs.regs[b];
		TRACE("ashr");
		cpu.asregs.regs[a] = av >> bv;
	      }
	      break;
	    case 0x2e: /* xor */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av, bv;
		TRACE("xor");
		av = cpu.asregs.regs[a];
		bv = cpu.asregs.regs[b];
		cpu.asregs.regs[a] = av ^ bv;
	      }
	      break;
	    case 0x2f: /* mul.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		unsigned av = cpu.asregs.regs[a];
		unsigned bv = cpu.asregs.regs[b];
		TRACE("mul.l");
		cpu.asregs.regs[a] = av * bv;
	      }
	      break;
	    case 0x30: /* swi */
	      {
		unsigned int inum = EXTRACT_WORD(pc+2);
		TRACE("swi");
		/* Set the special registers appropriately.  */
		cpu.asregs.sregs[2] = 3; /* MOXIE_EX_SWI */
	        cpu.asregs.sregs[3] = inum;
		switch (inum)
		  {
		  case 0x1: /* SYS_exit */
		    {
		      cpu.asregs.exception = SIGQUIT;
		      break;
		    }
		  case 0x2: /* SYS_open */
		    {
		      char fname[1024];
		      int mode = (int) convert_target_flags ((unsigned) cpu.asregs.regs[3]);
		      int perm = (int) cpu.asregs.regs[4];
		      int fd = open (fname, mode, perm);
		      sim_core_read_buffer (sd, scpu, read_map, fname,
					    cpu.asregs.regs[2], 1024);
		      /* FIXME - set errno */
		      cpu.asregs.regs[2] = fd;
		      break;
		    }
		  case 0x4: /* SYS_read */
		    {
		      int fd = cpu.asregs.regs[2];
		      unsigned len = (unsigned) cpu.asregs.regs[4];
		      char *buf = malloc (len);
		      cpu.asregs.regs[2] = read (fd, buf, len);
		      sim_core_write_buffer (sd, scpu, write_map, buf,
					     cpu.asregs.regs[3], len);
		      free (buf);
		      break;
		    }
		  case 0x5: /* SYS_write */
		    {
		      char *str;
		      /* String length is at 0x12($fp) */
		      unsigned count, len = (unsigned) cpu.asregs.regs[4];
		      str = malloc (len);
		      sim_core_read_buffer (sd, scpu, read_map, str,
					    cpu.asregs.regs[3], len);
		      count = write (cpu.asregs.regs[2], str, len);
		      free (str);
		      cpu.asregs.regs[2] = count;
		      break;
		    }
		  case 0xffffffff: /* Linux System Call */
		    {
		      unsigned int handler = cpu.asregs.sregs[1];
		      unsigned int sp = cpu.asregs.regs[1];

		      /* Save a slot for the static chain.  */
		      sp -= 4;

		      /* Push the return address.  */
		      sp -= 4;
		      wlat (scpu, opc, sp, pc + 6);
		
		      /* Push the current frame pointer.  */
		      sp -= 4;
		      wlat (scpu, opc, sp, cpu.asregs.regs[0]);

		      /* Uncache the stack pointer and set the fp & pc.  */
		      cpu.asregs.regs[1] = sp;
		      cpu.asregs.regs[0] = sp;
		      pc = handler - 6;
		    }
		  default:
		    break;
		  }
		pc += 4;
	      }
	      break;
	    case 0x31: /* div.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av = cpu.asregs.regs[a];
		int bv = cpu.asregs.regs[b];
		TRACE("div.l");
		cpu.asregs.regs[a] = av / bv;
	      }
	      break;
	    case 0x32: /* udiv.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		unsigned int av = cpu.asregs.regs[a];
		unsigned int bv = cpu.asregs.regs[b];
		TRACE("udiv.l");
		cpu.asregs.regs[a] = (av / bv);
	      }
	      break;
	    case 0x33: /* mod.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		int av = cpu.asregs.regs[a];
		int bv = cpu.asregs.regs[b];
		TRACE("mod.l");
		cpu.asregs.regs[a] = av % bv;
	      }
	      break;
	    case 0x34: /* umod.l */
	      {
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		unsigned int av = cpu.asregs.regs[a];
		unsigned int bv = cpu.asregs.regs[b];
		TRACE("umod.l");
		cpu.asregs.regs[a] = (av % bv);
	      }
	      break;
	    case 0x35: /* brk */
	      TRACE("brk");
	      cpu.asregs.exception = SIGTRAP;
	      pc -= 2; /* Adjust pc */
	      break;
	    case 0x36: /* ldo.b */
	      {
		unsigned int addr = EXTRACT_WORD(pc+2);
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		TRACE("ldo.b");
		addr += cpu.asregs.regs[b];
		cpu.asregs.regs[a] = rbat (scpu, opc, addr);
		pc += 4;
	      }
	      break;
	    case 0x37: /* sto.b */
	      {
		unsigned int addr = EXTRACT_WORD(pc+2);
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		TRACE("sto.b");
		addr += cpu.asregs.regs[a];
		wbat (scpu, opc, addr, cpu.asregs.regs[b]);
		pc += 4;
	      }
	      break;
	    case 0x38: /* ldo.s */
	      {
		unsigned int addr = EXTRACT_WORD(pc+2);
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		TRACE("ldo.s");
		addr += cpu.asregs.regs[b];
		cpu.asregs.regs[a] = rsat (scpu, opc, addr);
		pc += 4;
	      }
	      break;
	    case 0x39: /* sto.s */
	      {
		unsigned int addr = EXTRACT_WORD(pc+2);
		int a = (inst >> 4) & 0xf;
		int b = inst & 0xf;
		TRACE("sto.s");
		addr += cpu.asregs.regs[a];
		wsat (scpu, opc, addr, cpu.asregs.regs[b]);
		pc += 4;
	      }
	      break;
	    default:
	      opc = opcode;
	      TRACE("SIGILL1");
	      cpu.asregs.exception = SIGILL;
	      break;
	    }
	}

      insts++;
      pc += 2;

    } while (!cpu.asregs.exception);

  /* Hide away the things we've cached while executing.  */
  cpu.asregs.regs[PC_REGNO] = pc;
  cpu.asregs.insts += insts;		/* instructions done ... */
}

