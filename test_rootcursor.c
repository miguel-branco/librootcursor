#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "rootcursor.h"

int
main(int argc, char *argv[])
{
  char *fnames[] = {
      "/home/miguel/NTUP_TOPEL.00872780._000002.root.1",
      "/home/miguel/NTUP_TOPEL.00872780._000012.root.1" };
      
  double sum;

  Root *root = init_root(fnames, 2);
  RootTable *event = get_root_table(root, "physics", 0);
  RootTable *muon = get_root_table(root, "physics", 1);  
  RootTable *jet = get_root_table(root, "physics", 1);
  RootTable *electron = get_root_table(root, "physics", 1);

  printf("approx nr of events: %" PRId64 "\n", get_root_table_approx_size(event));
  printf("approx nr of muon: %" PRId64 "\n", get_root_table_approx_size(muon));
  printf("approx nr of jet: %" PRId64" \n", get_root_table_approx_size(jet));
  printf("approx nr of electron: %" PRId64 "\n", get_root_table_approx_size(electron));

  printf("event:\n");
  RootCursor *event_cursor = init_root_cursor(event, 2);
  set_root_cursor_attr(event_cursor, 0, "RunNumber", RootUInt);
  set_root_cursor_attr(event_cursor, 1, "lbn", RootUInt);
  open_root_cursor(event_cursor);
  while (advance_root_cursor(event_cursor))
  {
    int i;
    for (i = 0; i < 2; i++)
    {
      switch (get_root_cursor_attr_type(event_cursor, i))
      {
        case RootInt:
          printf("%d", get_int(event_cursor, i));
          break;
        case RootUInt:
          printf("%u", get_uint(event_cursor, i));
          break;
        case RootFloat:
          printf("%f", get_float(event_cursor, i));
          break;
        default:
          break;          
      }
      if (i != 1)
      {
        printf("\t");
      }      
    }
    printf("\n");
  }
  fini_root_cursor(event_cursor);

  printf("muon:\n");
  RootCursor *muon_cursor = init_root_cursor(muon, 2);
  set_root_cursor_attr(muon_cursor, 0, "mu_pt", RootFloat);
  set_root_cursor_attr(muon_cursor, 1, "mu_nPixelDeadSensors", RootInt);  
  open_root_cursor(muon_cursor);
  sum = 0.0;
  while (advance_root_cursor(muon_cursor))
  {
    int i;
    for (i = 0; i < 2; i++)
    {
      switch (get_root_cursor_attr_type(muon_cursor, i))
      {
        case RootInt:
          printf("%d", get_int(event_cursor, i));
          break;
        case RootUInt:
          printf("%u", get_uint(event_cursor, i));
          break;
        case RootFloat:
          printf("%f", get_float(event_cursor, i));
          break;
        default:
          break;          
      }
      if (i != 1)
      {
        printf("\t");
      }      
    }
    printf("\n");  
    sum += get_float(muon_cursor, 0);
  }
  fini_root_cursor(muon_cursor);
  printf("sum_att1:%lf\n", sum);
  
  printf("jet:\n");
  RootCursor *jet_cursor = init_root_cursor(jet, 2);
  set_root_cursor_attr(jet_cursor, 0, "jet_flavor_weight_SV1", RootFloat);
  set_root_cursor_attr(jet_cursor, 1, "jet_eta", RootFloat);  
  open_root_cursor(jet_cursor);
  sum = 0.0;
  while (advance_root_cursor(jet_cursor))
  {
    int i;
    for (i = 0; i < 2; i++)
    {
      switch (get_root_cursor_attr_type(jet_cursor, i))
      {
        case RootInt:
          printf("%d", get_int(event_cursor, i));
          break;
        case RootUInt:
          printf("%u", get_uint(event_cursor, i));
          break;
        case RootFloat:
          printf("%f", get_float(event_cursor, i));
          break;
        default:
          break;
      }
      if (i != 1)
      {
        printf("\t");
      }
    }
    printf("\n");  
    sum += get_float(jet_cursor, 1);
  }
  fini_root_cursor(jet_cursor);
  printf("sum_att2:%lf\n", sum);

  fini_root_table(electron);
  fini_root_table(jet);
  fini_root_table(muon);  
  fini_root_table(event);
  fini_root(root);
  
  return 0;
}
