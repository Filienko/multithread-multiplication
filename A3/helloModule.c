#include<linux/module.h>
#include<linux/sched/signal.h>
#include<linux/pid_namespace.h>
#include<asm/io.h>

// TOTALS structures
struct Stats {
  unsigned long total_con;
  unsigned long total_noncon;
  unsigned long total_pages;
};
struct task_struct;
struct Stats proc_count(void);

int virt2phys(struct task_struct* task, unsigned long virt)
{
  pgd_t *pgd;
  p4d_t *p4d;
  pud_t *pud;
  pmd_t *pmd;
  pte_t *pte;
  struct page *page;
  pgd = pgd_offset(task->mm, virt);
  unsigned long phys = 0;
  if (pgd_none(*pgd) || pgd_bad(*pgd))
      return 0;
  p4d = p4d_offset(pgd, virt);
  if (p4d_none(*p4d) || p4d_bad(*p4d))
      return 0;
  pud = pud_offset(p4d, virt);
  if (pud_none(*pud) || pud_bad(*pud))
      return 0;
  pmd = pmd_offset(pud, virt);
  if (pmd_none(*pmd) || pmd_bad(*pmd))
      return 0;
  if (!(pte = pte_offset_map(pmd, virt)))
      return 0;
  if (!(page = pte_page(*pte)))
      return 0;
  phys = page_to_phys(page);
  pte_unmap(pte);
  if (phys == 70368744173568)
    return 0;
  return 1;
}

int proc_init(void) {
  struct Stats counter_totals;
  counter_totals.total_con = 0;
  counter_totals.total_noncon = 0;
  counter_totals.total_pages = 0;

  printk(KERN_INFO "PROCESS REPORT:\n");
  printk(KERN_INFO "proc_id,proc_name,contig_pages,noncontig_pages,total_pages\n");
  counter_totals = proc_count();
  printk(KERN_INFO "TOTALS,, %lu,%lu,%lu\n",
    counter_totals.total_con,counter_totals.total_noncon,counter_totals.total_pages);
  return 0;
}
    

void proc_cleanup(void) {
  printk(KERN_INFO "helloModule: performing cleanup of module\n");
}

static struct Stats pages_count(struct task_struct* task)
{
  struct Stats pages_totals;
  pages_totals.total_con = 0;
  pages_totals.total_noncon = 0;
  pages_totals.total_pages = 0;
  unsigned long con_pages = 1;
  unsigned long total_pages = 0;
  struct vm_area_struct *vma = 0;
  unsigned long vpage;
  int prev_page_present = 0;
  if (task->mm && task->mm->mmap)
  {
    for (vma = task->mm->mmap; vma != NULL; vma = vma->vm_next)
    {
      for (vpage = vma->vm_start; vpage<vma->vm_end;vpage+=PAGE_SIZE)
      {
        
        //HIGH LEVEL LOGIC TO FIND NUMBER OF CONTINUOUS PAGES PER PROCESS
        if(prev_page_present != 0 && virt2phys(task,vpage) != 0)
        {
          //HIGH LEVEL LOGIC TO FIND TOTAL NUMBER OF PAGES PER PROCESS
          con_pages++;
          total_pages++;
        }
        else if (virt2phys(task,vpage) != 0)
        {
          total_pages++;
        }
        prev_page_present = virt2phys(task,vpage);
        //         //HIGH LEVEL LOGIC TO FIND NUMBER OF CONTINUOUS PAGES PER PROCESS
        // if ((vma->vm_next == NULL || vma->vm_next->vm_start != vma->vm_end)) 
        // {
        //   con_pages += (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
        // }
      }
    }
  }
  //Adjust if no pages exist, assume one entry is not sufficient to be counter contiuous
  if(con_pages == 1)
  {
    con_pages = 0;
  }
  pages_totals.total_con = con_pages;
  pages_totals.total_noncon = (total_pages - con_pages);
  pages_totals.total_pages = total_pages;

  return pages_totals;
}
struct Stats proc_count()
{
  struct Stats counter_totals;
  counter_totals.total_con = 0;
  counter_totals.total_noncon = 0;
  counter_totals.total_pages = 0;
  unsigned long total_pages =0;
  unsigned long total_con_pages =0;
  unsigned long total_noncon_pages =0;
  struct task_struct *thechild;
  for_each_process(thechild)
  {
    if(thechild->pid > 650)
    {
      //Process ID
      pid_t proc_id = thechild->pid;
      //Process name
      char *proc_name = thechild->comm;

      counter_totals = pages_count(thechild);

      //Total number of memory pages allocated for the process
      total_pages = total_pages + counter_totals.total_pages;
      //Total number of contiguously memory pages allocated for the process
      total_con_pages = total_con_pages + counter_totals.total_con;
      //Total number of non-contiguously memory pages allocated for the process
      total_noncon_pages = total_noncon_pages + counter_totals.total_noncon;

      printk(KERN_INFO "%lu, %s, %lu, %lu, %lu\n", proc_id, proc_name, counter_totals.total_con,counter_totals.total_noncon,counter_totals.total_pages);
    }
  }

  counter_totals.total_con = total_pages;
  counter_totals.total_noncon = total_noncon_pages;
  counter_totals.total_pages = total_con_pages;

 return counter_totals;
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
