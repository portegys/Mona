/*
LSTM (Long Short-Term Memory neural network)
simulator from Johannes Kepler University, Institute of Bioinformatics.
(www.bioinf.jku.at/software/lstm)
*/

#include <math.h>
#include <stdio.h>

#define max_blocks 10
#define max_size 10
#define max_units 80
#define max_inputs 10
#define max_outputs 10
#define max_sequence_length 500
#define max_training_size 501
#define max_test_size 501

FILE *fp1,*fp2;
char *outfile,*weightfile;
int 
  /*number inputs */
  in_mod,
  /*number outputs */
  out_mod,
  /*number hidden units */
  hid_mod, 
  /*number input and hidden units */
  hi_in_mod,
  /*number input and hidden units and gates and memory cells */
  cell_mod,
  /* number of memory cell blocks */
  num_blocks,
  /* Size of the memory cell blocks */
  block_size[max_blocks],
  /* number of all units */
  ges_mod,
  /* current sequence of the training set */
  examples,
  /* random number generator init */
  ran_sta,
  /* performing a test on a test  set? if ((epoch%test_aus==0)||(class_err<min_fehl)) */
  test_aus,
  min_fehl,
  /* write the weight matrix after w_out epochs */
  w_out,
  /* should the net still be learning --> learn=1 */
  learn,
  /* is stop to learn set? --> stop_learn=1 */
  stop_learn,
  /* max. number of trials to be performed */
  maxtrials,
  /* reset after each sequence? */
  sequ_reset,
  training_size,
  test_size,
  /* max. number of epochs before learning stop*/
  maxepoch,
  maxepoch_init,
  /* bias1==1 --> hidden units, with gates, and cells biased */
  bias1,
  /* bias2==1 --> output units biased */
  bias2,
  /* is target provided for the current input */
  targ,
  /* element in a sequence of the test set */
  element_t,
  /* number of sequences in test set */
  example_t,
  /* misclassifications per epoch */
  class_err,
  /* when to stop learning: wrong classifications per epoch < stop_class */
  stop_class,
  /* epochs to be learned after stop learning is set */
  ext_epochs,
  /* 1 if the current sequence has been processed correctly so far */
  seq_cor,
  /* 1 if end of the test set */
  test_end,
  /* weight_up == 1 then weight update */
  weight_up,
  /* w_up if w_up == 1 then weight update per sequence otherwise per epoch*/
  w_up,
  /* element in a sequence of the training set */
  element,
  /* number of sequences in training set */
  example,
  /* number of sequences seen by the net */
  numb_seq,
  /* number of epochs seen by the net */
  epoch,
  /* length of the training sequences */
  length[max_training_size],
  /* length of the test sequences */
  length_t[max_test_size];


double 
  /* weight matrix */
  W_mod[max_units][max_units],
  /* contribution to update of weight matrix */
  DW[max_units][max_units],
  /* input gates */
  Y_in[max_blocks],
  /* output gates */
  Y_out[max_blocks],
  /* new activation for all units */
  Yk_mod_new[max_units],
  /* old activation for all units */
  Yk_mod_old[max_units],
  /* function g for each cell */
  G[max_blocks][max_size],
  /* function h for each cell */
  H[max_blocks][max_size],
  /* internal state for each cell */
  S[max_blocks][max_size],
  /* output for each cell */
  Yc[max_blocks][max_size],
  /* derivative with respect to weights to input gate for each cell */
  SI[max_blocks][max_size][max_units],
  /* derivative with respect to weights to cell input for each cell */
  SC[max_blocks][max_size][max_units],
  /* initial input and output gate bias for each block */
  bias_inp[max_blocks],
  bias_out[max_blocks],
  /* learing rate */
  alpha, 
  /* interval of weight initialization is init_range*[-1,1] */
  init_range, 
  /* current target */
  target_a[max_units],
  /* matrix for storing the target per training element per
     seqence element per output component */
  tar[max_training_size][max_sequence_length][max_outputs],
  /* matrix for storing the input per training element per
seqence element per input component */
  inp[max_training_size][max_sequence_length][max_inputs],
  /* matrix for storing the target per test element per
seqence element per output component */
  tar_t[max_test_size][max_sequence_length][max_outputs],
  /* matrix for storing the input per test element per
seqence element per input component */
  inp_t[max_test_size][max_sequence_length][max_inputs],
  /* MSE per epoch */
  epoch_err,
  /* MSE per sequence */
  seq_err,
  /* when to stop learning: MSE per epoch < stop_mse */
  stop_mse,
  /* error for output units */
  error[max_units],
  /* max. allowed error per sequence for correct classification */
  seq_max; 

/* the output and weight files for 20 trials */
static char
*outf[] = {           "outa.txt",
		      "outb.txt",
		      "outc.txt",
		      "outd.txt",
		      "oute.txt",
		      "outf.txt",
		      "outg.txt",
		      "outh.txt",
		      "outi.txt",
		      "outj.txt",
		      "outk.txt",
		      "outl.txt",
		      "outm.txt",
		      "outn.txt",
		      "outo.txt",
		      "outp.txt",
		      "outq.txt",
		      "outr.txt",
		      "outs.txt",
		      "outt.txt"         },

  *weig[] = {           "weia.par",
			"weib.par",
			"weic.par",
			"weid.par",
			"weie.par",
			"weif.par",
			"weig.par",
			"weih.par",
			"weii.par",
			"weij.par",
			"weik.par",
			"weil.par",
			"weim.par",
			"wein.par",
			"weio.par",
			"weip.par",
			"weiq.par",
			"weir.par",
			"weis.par",
			"weit.par"         };

long random();
void srandom();

int seprand(k)
     int k;
{
  long l;
  int f;
  l = random();
  f = l % k;
  return(f);
}

void reset_net()
{
  int i,j,v,u;

  for (i=0;i<ges_mod;i++)
    {
      Yk_mod_new[i]=0.5;
      Yk_mod_old[i]=0.5;
    }
  for (u=0;u<num_blocks;u++)
    {
      for (v=0;v<block_size[u];v++)	
	{
	  S[u][v]=0;
	  G[u][v]=0;
	  H[u][v]=0;
	  Yc[u][v]=0.5;
	  for (j=0;j<cell_mod;j++)
	    {
	      SI[u][v][j]=0;
	      SC[u][v][j]=0;
	    }
	}
    }


}

void set_input_t()
{
  int i,j,k;
  double max;
  if (bias1==0)
    {
      for (i=0;i<in_mod;i++)
	{
	  Yk_mod_new[i]=inp_t[example_t][element_t][i];
	  Yk_mod_old[i]=Yk_mod_new[i];
	}
    }
  else
    {
      for (i=0;i<in_mod-1;i++)
	{
	  Yk_mod_new[i]=inp_t[example_t][element_t][i];
	  Yk_mod_old[i]=Yk_mod_new[i];
	}
      Yk_mod_new[in_mod-1]=1.0;
      Yk_mod_old[in_mod-1]=1.0;
    }

  max=0;
  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
    {
      target_a[j]=tar_t[example_t][element_t][j];
      if (fabs(tar_t[example_t][element_t][j])>max)
	max=fabs(tar_t[example_t][element_t][j]);
    }
  targ=1;
  if (max>1.0)
    targ=0;
}


void execute_act_test()
{
  set_input_t();
  element_t++;
  if (element_t>length_t[example_t])
    {
      element_t=0;
      example_t++;
      seq_cor=1;
      seq_err=0;
      if (sequ_reset==1)
	reset_net();
      if (example_t>test_size-1)
	{
	  test_end=1;
	}
      set_input_t();
      element_t++;
    }
}

void forward_pass()
{
  int i,j,u,v,k;
  double sum;
     
  /* ### hidden units ### */

  if (bias2==0)
    {
      for (i=in_mod;i<hi_in_mod;i++)
	{
	  sum = 0;
	  for (j=0;j<cell_mod;j++)
	    sum += W_mod[i][j]*Yk_mod_old[j];
	  Yk_mod_new[i] = 1/(1+exp(-sum));
	};
    }
  else
    {
      for (i=in_mod;i<hi_in_mod-1;i++)
	{
	  sum = 0;
	  for (j=0;j<cell_mod;j++)
	    sum += W_mod[i][j]*Yk_mod_old[j];
	  Yk_mod_new[i] = 1/(1+exp(-sum));
	};
      Yk_mod_new[hi_in_mod-1] = 1.0;
    }


  /* ### memory cells ### */

  i=hi_in_mod;
  for (u=0;u<num_blocks;u++)
    {
      /* input gate */
      sum = 0;
      for (j=0;j<cell_mod;j++)
	sum += W_mod[i][j]*Yk_mod_old[j];
      Y_in[u] = 1/(1+exp(-sum));
      Yk_mod_new[i]= Y_in[u];
      /* output gate */
      i++;
      sum = 0;
      for (j=0;j<cell_mod;j++)
	sum += W_mod[i][j]*Yk_mod_old[j];
      Y_out[u] = 1/(1+exp(-sum));
      Yk_mod_new[i]= Y_out[u];
      /* uth memory cell block */
      for (v=0;v<block_size[u];v++)	
	{
	  /* activation of function g of vth memory cell of block u  */
	  i++;
	  sum = 0;
	  for (j=0;j<cell_mod;j++)
	    sum += W_mod[i][j]*Yk_mod_old[j];
	  G[u][v] = 4.0/(1+exp(-sum))-2.0;
	  /* update internal state  */
	  S[u][v] = S[u][v]  + Y_in[u]*G[u][v];
	  /* activation function h */
	  H[u][v]= 2.0/(1+exp(-S[u][v]))-1.0;
	  /* activation of vth memory cell of block u */
	  Yc[u][v] = H[u][v]*Y_out[u];
	  Yk_mod_new[i] = Yc[u][v];
	};
      i++;
    };

  /* ### output units activation ### */

  if (targ==1) /* only if target for this input */
    {
      for (k=cell_mod;k<ges_mod;k++)
	{
	  /* hidden units input */
	  sum = 0;
	  for (i=in_mod;i<hi_in_mod;i++)
	    {
	      sum += W_mod[k][i]*Yk_mod_new[i];
	    };
	  /* memory cells input */
	  i=hi_in_mod;
	  for (u=0;u<num_blocks;u++)
	    {
	      i++;
	      for (v=0;v<block_size[u];v++)	
		{
		  i++;
		  sum += W_mod[k][i]*Yk_mod_new[i];
		};
	      i++;
	    }
	  /* activation */
	  Yk_mod_new[k] = 1/(1+exp(-sum));
	};
    }
}

void comp_err() 
{
  int k,j,maxout;
  double err,max;

  /* MSE */
  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
    {
      err=  error[j]*error[j];
    };
  seq_err+=err;
  epoch_err+=err;

  /* Maximal error output */

  max=0;
  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
    {
      if (fabs(error[j])>max)
	{
	  max=fabs(error[j]);
	  maxout=j;
	}
    };
  if ((seq_cor==1)&&(max>seq_max))
    {
      seq_cor=0;
      class_err++;
    }
}

void test()
{
  int i,k,j;
  int err,pass,total;
  int maxtarg,maxout;
  double maxval;
  err = pass = total = 0;
  element_t=0;
  example_t=0;
  epoch_err=0;
  class_err=0;
  seq_cor=1;
  seq_err=0;
  test_end=0;

  while (test_end==0)
    {

      /* executing the environment
	 and setting the input
	 */
      execute_act_test();
      if (test_end) break;

      /* forward pass */

      forward_pass();

      if (targ==1) /* only if target for this input */
	{
	  /* compute error */
          maxtarg = maxout = -1;
	  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
	    {
	      if (target_a[j] > 0.9) maxtarg = j;
	      if (maxout == -1 || Yk_mod_new[k] > maxval) { maxval = Yk_mod_new[k]; maxout = j; }
	      error[j]=  target_a[j] - Yk_mod_new[k];
	      //printf("%.2f/%.2f ", target_a[j],Yk_mod_new[k]);
	    };
	    //printf("\n");
            if (maxtarg >= 0 && maxtarg != maxout) err = 1;
            if (element_t==length_t[example_t])
            {
              total++;
              if (!err) pass++;
              //if (err) printf("error "); else printf("OK ");
              //printf("%d/%d\n",pass,total);
              err = 0;
            }

	  /* Training error */

	  comp_err();
	}

      /* set old activations */
      for (i=0;i<ges_mod;i++)
	{
	  Yk_mod_old[i] = Yk_mod_new[i];
        }

    }
    printf("%d / %d\n",pass,total);

  #ifdef NEVER
  fp1=fopen(outfile, "a");
  fprintf(fp1,"TEST: epochs:%d sequences:%d\n",epoch+1,numb_seq);
  fprintf(fp1,"TEST: MSE:%.4f\n",epoch_err/(1.0*test_size));
  fprintf(fp1,"TEST: misclassifications:%d (out of %d test examples)\n",class_err,test_size);
  fprintf(fp1,"\n");
  fclose(fp1);
  #endif
  
}

void output_epoch()
{
  #ifdef NEVER
  fp1=fopen(outfile, "a");
  fprintf(fp1,"epochs:%d sequences:%d\n",epoch+1,numb_seq);
  fprintf(fp1,"MSE:%.4f\n", epoch_err/(1.0*training_size));
  fprintf(fp1,"misclassifications:%d (out of %d training examples)\n",class_err,training_size);
  fprintf(fp1,"\n");
  fclose(fp1);
  #endif
}

void weight_out()
{
  #ifdef NEVER
  int i,j;
  fp2 = fopen(weightfile, "w");
  fprintf(fp2,"anz:%d\n",numb_seq);
  for (i=0;i<ges_mod;i++)
    {
      for (j=0;j<ges_mod;j++)
	fprintf(fp2,"(%.2d,%.2d): %.3f ",i,j,W_mod[i][j]);
      fprintf(fp2,"\n");
    };
  fprintf(fp2,"\n");
  fclose(fp2);
  #endif
}

void set_input()
{
  int i,j,k;
  double max;
  if (bias1==0)
    {
      for (i=0;i<in_mod;i++)
	{
	  Yk_mod_new[i]=inp[example][element][i];
	  Yk_mod_old[i]=Yk_mod_new[i];
	}
    }
  else
    {
      for (i=0;i<in_mod-1;i++)
	{
	  Yk_mod_new[i]=inp[example][element][i];
	  Yk_mod_old[i]=Yk_mod_new[i];
	}
      Yk_mod_new[in_mod-1]=1.0;
      Yk_mod_old[in_mod-1]=1.0;
    }

  max=0;
  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
    {
      target_a[j]=tar[example][element][j];
      if (fabs(tar[example][element][j])>max)
	max=fabs(tar[example][element][j]);
    }
  /* is there a target for this input */
  targ=1;
  if (max>1.0)
    targ=0;
}

void execute_act()
{
  /* setting the net input */
  set_input();
  /* next element in the sequence */
  element++;
  /* sequence end */
  if (element>length[example])
    {
      /* first element in a sequence */
      element=0;
      /* counting the number of sequences in a epoch */
      example++;
      /* counting the number of sequences the net has seen */
      numb_seq++;
      /* resetting the error for the current sequence */
      seq_cor=1;
      /* resetting the error per sequence */
      seq_err=0;
      /* reset after each sequence? */
      if (sequ_reset==1)
	reset_net();
      /* weight update after each sequence? */
      if (w_up==1)
	weight_up=1;
      /* end of an epoch ? */
      if (example>training_size-1)
	{
	  /* MSE and misclassifications output for training set */
	  output_epoch();
	  /* when to stop learning */
	  if (stop_learn==0)
	    {
	      if ((epoch_err<stop_mse)||(class_err<stop_class))
		{
		  stop_learn=1;
		  maxepoch=epoch+ext_epochs;
		}
	    }
	  /* weight update after each epoch? */
	  if (w_up!=1)
	    weight_up=1;
          #ifdef NEVER
	  /* performing a test on a test  set? */
	  if ((epoch%test_aus==0)||(class_err<min_fehl))
	    {
	      reset_net();
	      test();
	      reset_net();
	      seq_cor=1;
	      seq_err=0;
	    }
	    #endif
	  /* first sequence in the training set */
	  example=0;
	  /* counting the epochs */
	  epoch++;
	  /* resetting the error per epoch */
	  epoch_err=0;
	  /* resetting the misclassifications per epoch */
	  class_err=0;
	  /* Write weight matrix */
	  if (epoch%w_out==0)
	    weight_out();

	}
      set_input();
      element++;
    }
}

void initia()
{
  int i, j, u, v;

  example=0;
  epoch=0;
  epoch_err=0;
  class_err=0;
  numb_seq=0;
  seq_cor=1;
  seq_err=0;
  weight_up=0;
   

  /* weight initialization */
  for (i=0;i<ges_mod;i++)
    {
      for (j=0;j<ges_mod;j++)
	{
	  W_mod[i][j] = (seprand(2000) - 1000);
	  W_mod[i][j] /= 1000.0;
	  W_mod[i][j]*=init_range;
	  DW[i][j]=0;
	};

    };

  /* gates bias initalization */
  if (bias1==1)
    {
      i=hi_in_mod;
      for (u=0;u<num_blocks;u++)
	{
	  W_mod[i][in_mod-1]+= bias_inp[u];
	  i++;
	  W_mod[i][in_mod-1]+= bias_out[u];
	  for (v=0;v<block_size[u];v++)	
	    {
  	      i++;
	    };
	  i++;
	};
    }

  /* reset activations and derivatives */

  reset_net();



}




void derivatives()
{
  int u,v,j;
  for (u=0;u<num_blocks;u++)
    {
      for (v=0;v<block_size[u];v++)	
	{
	  /* weights to input gate */
	  for (j=0;j<cell_mod;j++)
	    SI[u][v][j]=SI[u][v][j] + G[u][v]*(1.0-Y_in[u])*Y_in[u]*Yk_mod_old[j];
	    /* weights to cell input */
	    for (j=0;j<cell_mod;j++)
	      SC[u][v][j]=SC[u][v][j] + Y_in[u]*(0.25*(2.0-G[u][v])*(2.0+G[u][v]))*Yk_mod_old[j];
	};
    };
}


void backward_pass()
{
  int k,i,j,u,v;
  double sum,
    e[max_units],
    ec[max_blocks][max_size],
    eo[max_blocks],
    es[max_blocks][max_size];
	
  /* output units */

  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
    {
      e[k]=error[j]*(1.0-Yk_mod_new[k])*Yk_mod_new[k];
      /* weight update contribution */
      for (i=in_mod;i<hi_in_mod;i++)
	{
	  DW[k][i] += alpha*e[k]*Yk_mod_new[i];
	};
      i=hi_in_mod;
      for (u=0;u<num_blocks;u++)
	{
	  i++;
	  for (v=0;v<block_size[u];v++)	
	    {
	      i++;
	      DW[k][i] += alpha*e[k]*Yk_mod_new[i];
	    };
	  i++;
	}
          
    };
  /* hidden units */

  for (i=in_mod;i<hi_in_mod;i++)
    {
      sum=0;
      for (k=cell_mod;k<ges_mod;k++)
	sum+= W_mod[k][i]*e[k];
      e[i]=sum*(1.0-Yk_mod_new[i])*Yk_mod_new[i];
      /* weight update contribution */
      for (j=0;j<cell_mod;j++)
	{
	  DW[i][j] += alpha*e[i]*Yk_mod_old[j];
	};
    }


  /* error to memory cells ec[][] and internal states es[][] */

  i=hi_in_mod;
  for (u=0;u<num_blocks;u++)
    {
      i++;
      for (v=0;v<block_size[u];v++)	
	{
	  i++;
	  sum = 0;
	  for (k=cell_mod;k<ges_mod;k++)
	    sum+= W_mod[k][i]*e[k];
	  ec[u][v]=sum;
	  es[u][v]=Y_out[u]*(0.5*(1.0+H[u][v])*(1.0-H[u][v]))*sum;
	};
      i++;
    };

  /* output gates */

  for (u=0;u<num_blocks;u++)
    {
      sum=0;
      for (v=0;v<block_size[u];v++)	
	{
	  sum+= H[u][v]*ec[u][v];
	};
      eo[u]=sum*(1.0-Y_out[u])*Y_out[u];
    };


  /* Derivatives of the internal state */

  derivatives();


  /* updates for weights to input and output gates and memory cells */
  i=hi_in_mod;
  for (u=0;u<num_blocks;u++)
    {
      for (j=0;j<cell_mod;j++)
	{
	  sum = 0;
	  for (v=0;v<block_size[u];v++)	
	    {
	      sum += es[u][v]*SI[u][v][j];  
	    }
	  DW[i][j] += alpha*sum;
	}
      i++;
      for (j=0;j<cell_mod;j++)
	{
	  DW[i][j]+= alpha*eo[u]*Yk_mod_old[j];
	}
      for (v=0;v<block_size[u];v++)	
	{
	  i++;
	  for (j=0;j<cell_mod;j++)
	    DW[i][j]+= alpha*es[u][v]*SC[u][v][j];
	};
      i++;
    }

}

void weight_update()
{
  int i,j,k,u,v;


  /* output units */

  for (k=cell_mod,j=0;k<ges_mod;k++,j++)
    {
      for (i=in_mod;i<hi_in_mod;i++)
	{
	  W_mod[k][i] += DW[k][i];
	  DW[k][i] = 0;
	};
      i=hi_in_mod;
      for (u=0;u<num_blocks;u++)
	{
	  i++;
	  for (v=0;v<block_size[u];v++)	
	    {
	      i++;
	      W_mod[k][i] += DW[k][i];
	      DW[k][i] = 0;
	    };
	  i++;
	}
          
    };
  /* hidden units */

  for (i=in_mod;i<hi_in_mod;i++)
    {
      for (j=0;j<cell_mod;j++)
	{
	  W_mod[i][j] += DW[i][j];
	  DW[i][j] = 0;
	};
    }

  /* memory cells with gates */
  i=hi_in_mod;
  for (u=0;u<num_blocks;u++)
    {
      for (j=0;j<cell_mod;j++)
	{
	  W_mod[i][j] += DW[i][j];
	  DW[i][j] = 0;
	}
      i++;
      for (j=0;j<cell_mod;j++)
	{
	  W_mod[i][j] += DW[i][j];
	  DW[i][j] = 0;
	}
      for (v=0;v<block_size[u];v++)	
	{
	  i++;
	  for (j=0;j<cell_mod;j++)
	    {
	      W_mod[i][j] += DW[i][j];
	      DW[i][j] = 0;
	    }
	};
      i++;
    }

}


void getpars()
{
  int j,u,v,dummy,corr[50],corr1[4][30],sav,max_b_size;
  char *wrong1[] = {
    "number inputs: ?",
    "number outputs: ?",
    "output layer biased: ?",
    "number hidden units: ?",
    "hidden layer biased: ?",
    "number memory cell blocks: ?",
    "###here definitions for each block###",
    "###end definitions for each block###",
    "learning rate: ?",
    "max. error for correct sequence: ?",
    "half interval length for intialization: ?",
    "performing test after ? epochs: ?",
    "performing test after fewer than ? wrong classifications on training set: ?",
    "write weight after ? epochs: ?",
    "max. number of trials: ?",
    "stop learning once MSE per epoch <: ?",
    "stop learning once wrong classifications per epoch <: ?",
    "epochs to be learned after stop learning is set: ?",
    "initialization of random generator: ?",
    "reset the net after each sequence?: ?",
    "weight update after sequence or epoch?: ?",
    "max. number of epochs: ?",
    "size of training set: ?",
    "size of test set: ?" } ,
 *wrong2[] = {
    "**block number: ?**",
    "block size: ?",
    "initial input gate bias: ?",
    "initial output gate bias: ?" };


  FILE *fp5;

  for (u=0;u<50;u++)
    {
      corr[u]=1;
    }
  for (v=0;v<4;v++)
    for (u=0;u<30;u++)
      {
	corr1[v][u]=1;
      }

  v=0;
  fp5=fopen("lstmpars.txt","r");
  corr[v]=fscanf(fp5,"number inputs: %d\n",&in_mod);
  /* are the maximal ranges correct? */
  if (in_mod>max_inputs)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_inputs at begin\n");
      printf("of the program file greater or equal %d and then\n",in_mod);
      printf("compile the program again.\n");
      exit(0);
    }
  v++;
  corr[v]=fscanf(fp5,"number outputs: %d\n",&out_mod);
  /* are the maximal ranges correct? */
  if (out_mod>max_outputs)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_outputs at begin\n");
      printf("of the program file greater or equal %d and then\n",out_mod);
      printf("compile the program again.\n");
      exit(0);
    }
  v++;
  corr[v]=fscanf(fp5,"output layer biased: %d\n",&bias2);
  v++;
  corr[v]=fscanf(fp5,"number hidden units: %d\n",&hid_mod);
  v++;
  corr[v]=fscanf(fp5,"hidden layer biased: %d\n",&bias1);
  v++;
  corr[v]=fscanf(fp5,"number memory cell blocks: %d\n",&num_blocks);
  /* are the maximal ranges correct? */
  if (num_blocks>max_blocks)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_blocks at begin\n");
      printf("of the program file greater or equal %d and then\n",num_blocks);
      printf("compile the program again.\n");
      exit(0);
    }
  v++;
  corr[v]=fscanf(fp5,"###here definitions for each block###\n");
  corr[v]=1;
  v++;
  sav=v;
  max_b_size=0;
  for (u=0;u<num_blocks;u++)
    {
      corr1[0][u]=fscanf(fp5,"**block number: %d**\n",&dummy);
      corr1[1][u]=fscanf(fp5,"block size: %d\n",&block_size[u]);
      if (block_size[u]>max_b_size)
	max_b_size=block_size[u];
      if (bias1==1)
	{
	  corr1[2][u]=fscanf(fp5,"initial input gate bias: %lf\n",&bias_inp[u]);
	  corr1[3][u]=fscanf(fp5,"initial output gate bias: %lf\n",&bias_out[u]);
	}
    }
  /* are the maximal ranges correct? */
  if (max_b_size>max_size)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_size at begin\n");
      printf("of the program file greater or equal %d and then\n",max_b_size);
      printf("compile the program again.\n");
      exit(0);
    }
  corr[v]=fscanf(fp5,"###end definitions for each block###\n");
  corr[v]=1;
  v++;
  corr[v]=fscanf(fp5,"learning rate: %lf\n",&alpha);
  v++;
  corr[v]=fscanf(fp5,"max. error for correct sequence: %lf\n",&seq_max);
  v++;
  corr[v]=fscanf(fp5,"half interval length for intialization: %lf\n",&init_range);
  v++;
  corr[v]=fscanf(fp5,"performing test after ? epochs: %d\n",&test_aus);
  v++;
  corr[v]=fscanf(fp5,"performing test after fewer than ? wrong classifications on training set: %d\n",&min_fehl);
  v++;
  corr[v]=fscanf(fp5,"write weight after ? epochs: %d\n",&w_out);
  v++;
  corr[v]=fscanf(fp5,"max. number of trials: %d\n",&maxtrials);
  v++;
  corr[v]=fscanf(fp5,"stop learning once MSE per epoch <: %lf\n",&stop_mse);
  v++;
  corr[v]=fscanf(fp5,"stop learning once wrong classifications per epoch <: %d\n",&stop_class);
  v++;
  corr[v]=fscanf(fp5,"epochs to be learned after stop learning is set: %d\n",&ext_epochs);
  v++;
  corr[v]=fscanf(fp5,"initialization of random generator: %d\n",&ran_sta);
  v++;
  corr[v]=fscanf(fp5,"reset the net after each sequence?: %d\n",&sequ_reset);
  v++;
  corr[v]=fscanf(fp5,"weight update after sequence or epoch?: %d\n",&w_up);
  v++;
  corr[v]=fscanf(fp5,"max. number of epochs: %d\n",&maxepoch_init);
  v++;
  corr[v]=fscanf(fp5,"size of training set: %d\n",&training_size);
  /* are the maximal ranges correct? */
  if (training_size>max_training_size)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_training_size at begin\n");
      printf("of the program file greater or equal %d and then\n",training_size);
      printf("compile the program again.\n");
      exit(0);
    }
  v++;
  corr[v]=fscanf(fp5,"size of test set: %d\n",&test_size);
  /* are the maximal ranges correct? */
  if (test_size>max_test_size)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_test_size at begin\n");
      printf("of the program file greater or equal %d and then\n",test_size);
      printf("compile the program again.\n");
      exit(0);
    }
  v++;
  fclose(fp5);

  for (u=0;u<sav;u++)
    {
      if (corr[u]==0)
	{
	  printf("Error in lstmpars.txt at line:\n");
	  printf("%s\n",wrong1[u]);
	  exit(0);
	}
    }
  for (u=0;u<num_blocks;u++)
    for (j=0;j<4;j++)
      {
	if (corr1[j][u]==0)
	  {
	    printf("Error in lstmpars.txt at block %d 's definition at line:\n",u+1);
	    printf("%s\n",wrong2[j]);
	    exit(0);
	  }
      }
  for (u=sav;u<v;u++)
    {
      if (corr[u]==0)
	{
	  printf("Error in lstmpars.txt at line:\n");
	  printf("%s\n",wrong1[u]);
	  exit(0);
	}
    }
}

void write_info()
{

      printf("Perhaps an error occurred during reading the file\n");
      printf("lstmtest.txt ---> stopping the program.\n");
      printf("There should be %d input components and\n",in_mod);
      printf("%d target components per line. Makes %d real\n",out_mod,in_mod+out_mod);
      printf("values per line.\n");
      printf("An input sequence ends with a line\n");
      printf("where the first value (out of %d values in\n",in_mod+out_mod);
      printf("the line) is greater than 10.0\n");
      printf("The file ends with an extra line\n");
      printf("where the first value (out of %d values in\n",in_mod+out_mod);
      printf("the line) is greater than 10.0\n");
}


void getsets()
{
  FILE *fp3,*fp4;
  int end_seq,elm,end,trains,i;


  fp3=fopen("lstmtrain.txt","r");
  end=0;
  trains=0;
  while (end==0)
    {
      end_seq=0;
      elm=0;
      while (end_seq==0)
	{
	  for (i=0;i<in_mod;i++)
	    {
	      fscanf(fp3,"%lf ",&inp[trains][elm][i]);
	    }
	  for (i=0;i<out_mod;i++)
	    {
	      fscanf(fp3,"%lf ",&tar[trains][elm][i]);
	    }
	  fscanf(fp3,"\n");
	  /* sequence ends with first input component greater 10.0 */
	  if (fabs(inp[trains][elm][0])>10.0)
	    {
	      /* if 2 successive sequences have a first input component greater
		 than 10.0, then training set is finished */ 
	      if (elm==0)
		{
		  end=1;
		}
	      else
		{
		  elm--;
		}
	      end_seq=1;
	    }
	  elm++;
	  if (elm>max_sequence_length)
	    {
	      printf("Program terminated!\n");
	      printf("You have to set the constant max_sequence_length at begin\n");
	      printf("of the program file greater or equal maximal sequence\n");
	      printf("length (>= %d) and then compile the program again.\n",elm);
	      exit(0);
	    }
	}
      length[trains]=elm;
      trains++;
      if (end==1)
	trains--;
    }
  fclose(fp3);
  if (trains!=training_size)
    {
      printf("Training set size in parameter file: %d.\n",training_size);
      printf("Training set size detected: %d.\n",trains);
      write_info();
      exit(0);
    }

  fp4=fopen("lstmtest.txt","r");
  end=0;
  trains=0;
  while (end==0)
    {
      end_seq=0;
      elm=0;
      while (end_seq==0)
	{
	  for (i=0;i<in_mod;i++)
	    {
	      fscanf(fp4,"%lf ",&inp_t[trains][elm][i]);
	    }
	  for (i=0;i<out_mod;i++)
	    {
	      fscanf(fp4,"%lf ",&tar_t[trains][elm][i]);
	    }
	  fscanf(fp3,"\n");
	  if (fabs(inp_t[trains][elm][0])>10.0)
	    {
	      if (elm==0)
		{
		  end=1;
		}
	      else
		{
		  elm--;
		}
	      end_seq=1;
	    }
	  elm++;

	  if (elm>max_sequence_length)
	    {
	      printf("Program terminated!\n");
	      printf("You have to set the constant max_sequence_length at begin\n");
	      printf("of the program file greater or equal maximal sequence\n");
	      printf("length (>= %d) and then compile the program again.\n",elm);
	      exit(0);
	    }
	}
      length_t[trains]=elm;
      trains++;
      if (end==1)
	trains--;
    }
  fclose(fp4);
  if (trains!=test_size)
    {
      printf("Test set size in parameter file: %d.\n",test_size);
      printf("Test set size detected: %d.\n",trains);
      write_info();
      exit(0);
    }
}

int main()
{

  int i, j, k,
    trialnr;

    
  /* input pars */

  getpars();

  /* input training set and test set */

  getsets();

  if (maxtrials>20)
    maxtrials=20;

  if (bias1==1)
    in_mod++;
  if (bias2==1)
    hid_mod++;
  hi_in_mod = in_mod+hid_mod;
  cell_mod=hi_in_mod;
  for (i=0;i<num_blocks;i++)
    cell_mod+=(2+block_size[i]);
  ges_mod = cell_mod+out_mod;

  if (ges_mod>max_units)
    {
      printf("Program terminated!\n");
      printf("You have to set the constant max_units at begin\n");
      printf("of the program file greater or equal %d and then\n",ges_mod);
      printf("compile the program again.\n");
      exit(0);
    }


  srandom(ran_sta);
  for (trialnr=0;trialnr<maxtrials;trialnr++)
    {


      outfile = outf[trialnr];

      weightfile = weig[trialnr];


      #ifdef NEVER
      fp1 = fopen(outfile, "w");
      fprintf(fp1,"Trial Nr.:%.1d\n",trialnr);
      fclose(fp1);

      fp2 = fopen(weightfile, "w");
      fprintf(fp2,"Trial Nr.:%.1d\n",trialnr);
      fclose(fp2);
      #endif

      initia();

      examples=0;
      epoch=0;

      maxepoch=maxepoch_init;

      stop_learn=0;
      learn = 1;

      while (learn == 1)
	{
      


	  /* executing the environment
	     and setting the input
	     */
	  execute_act();

	  /* stop if maxepoch reached */
	  if (epoch>=maxepoch)
	  {
	    learn=0;
	    break;
	  }

	  /* forward pass */

	  forward_pass();


	  if (targ==1) /* only if target for this input */
	    {
	      /* compute error */

	      for (k=cell_mod,j=0;k<ges_mod;k++,j++)
		{
		  error[j]=  target_a[j] - Yk_mod_new[k];
		};

	      /* Training error */

	      comp_err();
	    }



	  /* backward pass */

	  if (targ==1) /* only if target for this input */
	    {
	      backward_pass();
	    }
	  else
	    {
	      derivatives();
	    }
	

	  /* set old activations */
	  for (i=0;i<ges_mod;i++)
	    {
	      Yk_mod_old[i] = Yk_mod_new[i];
	    }


	  /* update weights */

	  if (weight_up==1)
	    {
	      weight_up=0;
	      weight_update();
	    }


	}
      weight_out();
      test();
    }

  return 0;
}










