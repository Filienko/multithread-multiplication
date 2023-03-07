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

struct Stats proc_count(void);

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

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);

static struct Stats pages_count(struct task_struct* task)
{
  struct Stats pages_totals;
  pages_totals.total_con = 0;
  pages_totals.total_noncon = 0;
  pages_totals.total_pages = 0;
  int temp_con = 0;
  int temp_noncon = 0;
  struct vm_area_struct *vma = 0;
  unsigned long vpage;
  if (task->mm && task->mm->mmap)
    for (vma = task->mm->mmap; vma; vma = vma->vm_next)
      for (vpage = vma->vm_start; vpage<vma->vm_end;vpage+=PAGE_SIZE)
      {
        if(virt2phys(task,vpage) != 0)
        {
          //HIGH LEVEL LOGIC
          temp_con += 1;
          pages_totals.total_noncon = (((pages_totals.total_noncon) >= (temp_noncon)) ? (pages_totals.total_noncon) : (temp_noncon));
          temp_noncon = 0;
        }
        else
        {
          //HIGH LEVEL LOGIC
          temp_noncon += 1;
          pages_totals.total_con = (((pages_totals.total_con) >= (temp_con)) ? (pages_totals.total_con) : (temp_con));
          temp_con = 0;
        }
      }
  return pages_totals;
}
    

void proc_cleanup(void) {
  printk(KERN_INFO "helloModule: performing cleanup of module\n");
}

struct Stats proc_count()
{
  struct Stats counter_totals;
  counter_totals.total_con = 0;
  counter_totals.total_noncon = 0;
  counter_totals.total_pages = 0;

  struct task_struct *thechild;
  for_each_process(thechild)
  {
    if(thechild->pid > 650)
    {
      pages_count(thechild);
      //Process ID
      pid_t proc_id = thechild->pid;
      //Process name
      char *proc_name = thechild->comm;
      //Total number of memory pages allocated for the process
      unsigned long total_pages = thechild->mm->total_vm;
      //Total number of contiguously memory pages allocated for the process
      unsigned long total_con_pages = total_pages - thechild->mm->nr_ptes;
      //Total number of non-contiguously memory pages allocated for the process
      unsigned longt total_noncon_pages = thechild->mm->nr_ptes;

      printk(KERN_INFO "%lu, %s, %lu, %lu, %lu\n", proc_id, proc_name, total_con_pages,total_noncon_pages,total_pages);
      counter_totals.total_con += total_con_pages;
      counter_totals.total_noncon += total_noncon_pages;
      counter_totals.total_pages += total_pages;
    }
  }
 return counter_totals;
}

int virt2phys(task_struct* task, unsigned long virt)
{
  pgd_t *p4d;
  pgd_t *pud;
  pgd_t *pmd;
  pgd_t *pte;
  struct page *page;
  pgd_t *pgd = pgd_offset(task->mm, virt);

  if (pgd_none(*pgd) || pgd_bad(*pgd))
      return 0;
  p4d = p4d_offset(p4d, virt);
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
  return phys;
}
