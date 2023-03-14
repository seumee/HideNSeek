import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class GameOptions extends JPanel{
    Values values;
    Values.gameOptions options = new Values.gameOptions();
    JPanel[] panels = new JPanel[7];
    ButtonGroup[] bGroups = new ButtonGroup[6];
    JRadioButton[] nOfTagger = new JRadioButton[3];
    JRadioButton[] gameSpeed = new JRadioButton[5];
    JRadioButton[] sight = new JRadioButton[5];
    JRadioButton[] speed = new JRadioButton[5];
    JRadioButton[] catchCooldown = new JRadioButton[5];
    JRadioButton[] blockCooldown = new JRadioButton[5];
    JButton resetBtn = new JButton("Reset Options");
    JButton okBtn = new JButton("OK");
    public GameOptions(Values values) {
        this.values = values;
        setUIs();
        Init();
    }

    private void Init() {
        setNofTagger();
        setGameSpeed();
        setSightDisadvantage();
        setTaggerSpeed();
        setCatchCooldown();
        setBlockCooldown();
        optionChangeListener.actionPerformed(null);
    }
    private void setUIs() {
        values.readValues.readListener = optionChangeListener;
        setLayout(new GridLayout(7, 1, 60, 0));
        setSize(600, 510);
        String[] titles = {"Number Of Tagger", "Game Speed", "Sight Disadvantage", "Tagger Speed", "Catch Cooldown", "Block Cooldown"};
        for(int i = 0; i < 6; i++) {
            bGroups[i] = new ButtonGroup();
            panels[i] = new JPanel();
            panels[i].setBorder(BorderFactory.createTitledBorder(titles[i]));
            add(panels[i]);
        }
        panels[6] = new JPanel();
        panels[6].setBorder(BorderFactory.createEmptyBorder(20, 0, 0, 0));
        add(panels[6]);

        resetBtn.addActionListener(setToDefault);
        resetBtn.setPreferredSize(new Dimension(150, 30));
        resetBtn.setFocusable(false);
        panels[6].add(resetBtn);

        okBtn.addActionListener(changeOptions);
        okBtn.setPreferredSize(new Dimension(150, 30));
        okBtn.setFocusable(false);
        panels[6].add(okBtn);
    }

    ActionListener NofTaggerListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.numberOfTagger = Integer.valueOf(((JRadioButton)e.getSource()).getText());
        }
    };
    ActionListener GameSpeedListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.gameSpeed = Integer.valueOf(((JRadioButton)e.getSource()).getText());
        }
    };
    ActionListener SightDisadvantageListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.sight_disadvantage = Integer.valueOf(((JRadioButton)e.getSource()).getText());
        }
    };
    ActionListener TaggerSpeedListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.taggerSpeed = Integer.valueOf(((JRadioButton)e.getSource()).getText());
        }
    };
    ActionListener CatchCooldownListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.catch_cooldown = Integer.valueOf(((JRadioButton)e.getSource()).getText());
        }
    };
    ActionListener BlockCooldownListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.block_cooldown = Integer.valueOf(((JRadioButton)e.getSource()).getText());
        }
    };

    private void setNofTagger() {
        for (int i = 0; i < nOfTagger.length; i++) {
            nOfTagger[i] = new JRadioButton(String.valueOf(i));
            nOfTagger[i].setFocusable(false);
            bGroups[0].add(nOfTagger[i]);
            panels[0].add(nOfTagger[i]);
            nOfTagger[i].addActionListener(NofTaggerListener);
        }
    }
    private void setGameSpeed() {
        for (int i = 0; i < gameSpeed.length; i++) {
            gameSpeed[i] = new JRadioButton(String.valueOf(80+i*10));
            gameSpeed[i].setFocusable(false);
            bGroups[1].add(gameSpeed[i]);
            panels[1].add(gameSpeed[i]);
            gameSpeed[i].addActionListener(GameSpeedListener);
        }
    }
    private void setSightDisadvantage() {
        for (int i = 0; i < sight.length; i++) {
            sight[i] = new JRadioButton(String.valueOf(10+i*10));
            sight[i].setFocusable(false);
            bGroups[2].add(sight[i]);
            panels[2].add(sight[i]);
            sight[i].addActionListener(SightDisadvantageListener);
        }
    }
    private void setTaggerSpeed() {
        for (int i = 0; i < speed.length; i++) {
            speed[i] = new JRadioButton(String.valueOf(100+i*10));
            speed[i].setFocusable(false);
            bGroups[3].add(speed[i]);
            panels[3].add(speed[i]);
            speed[i].addActionListener(TaggerSpeedListener);
        }
    }
    private void setCatchCooldown() {
        for (int i = 0; i < catchCooldown.length; i++) {
            catchCooldown[i] = new JRadioButton(String.valueOf(i*5));
            catchCooldown[i].setFocusable(false);
            bGroups[4].add(catchCooldown[i]);
            panels[4].add(catchCooldown[i]);
            catchCooldown[i].addActionListener(CatchCooldownListener);
        }
    }
    private void setBlockCooldown() {
        for (int i = 0; i < blockCooldown.length; i++) {
            blockCooldown[i] = new JRadioButton(String.valueOf(10+i*5));
            blockCooldown[i].setFocusable(false);
            bGroups[5].add(blockCooldown[i]);
            panels[5].add(blockCooldown[i]);
            blockCooldown[i].addActionListener(BlockCooldownListener);
        }
    }

    private void setToDefaultOptions() {
        options.numberOfTagger = 2;
        options.gameSpeed = 100;
        options.sight_disadvantage = 70;
        options.taggerSpeed = 120;
        options.catch_cooldown = 10;
        options.block_cooldown = 20;
    }
    ActionListener setToDefault = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            setToDefaultOptions();
            nOfTagger[1].setSelected(true);
            gameSpeed[2].setSelected(true);
            sight[2].setSelected(true);
            speed[2].setSelected(true);
            catchCooldown[2].setSelected(true);
            blockCooldown[2].setSelected(true);
            changeOptions.actionPerformed(null);
        }
    };
    ActionListener changeOptions = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            values.writeValues.msgType = MsgType.MSG_ROOM_CFG;
            values.staticValues.options.numberOfTagger = options.numberOfTagger;
            values.staticValues.options.gameSpeed = options.gameSpeed;
            values.staticValues.options.sight_disadvantage = options.sight_disadvantage;
            values.staticValues.options.taggerSpeed = options.taggerSpeed;
            values.staticValues.options.catch_cooldown = options.catch_cooldown;
            values.staticValues.options.block_cooldown = options.block_cooldown;
            values.writeValues.writeListener.actionPerformed(null);
        }
    };
    ActionListener optionChangeListener = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            options.numberOfTagger = values.staticValues.options.numberOfTagger;
            options.gameSpeed = values.staticValues.options.gameSpeed;
            options.sight_disadvantage = values.staticValues.options.sight_disadvantage;
            options.taggerSpeed = values.staticValues.options.taggerSpeed;
            options.catch_cooldown = values.staticValues.options.catch_cooldown;
            options.block_cooldown = values.staticValues.options.block_cooldown;

            for(int i = 0; i < nOfTagger.length; i++) {
                if(Integer.valueOf(nOfTagger[i].getText()) == values.staticValues.options.numberOfTagger) {
                    nOfTagger[i].setSelected(true);
                }
            }
            for(int i = 0; i < gameSpeed.length; i++) {
                if(Integer.valueOf(gameSpeed[i].getText()) == values.staticValues.options.gameSpeed) {
                    gameSpeed[i].setSelected(true);
                }
            }
            for(int i = 0; i < sight.length; i++) {
                if(Integer.valueOf(sight[i].getText()) == values.staticValues.options.sight_disadvantage) {
                    sight[i].setSelected(true);
                }
            }
            for(int i = 0; i < speed.length; i++) {
                if(Integer.valueOf(speed[i].getText()) == values.staticValues.options.taggerSpeed) {
                    speed[i].setSelected(true);
                }
            }
            for(int i = 0; i < catchCooldown.length; i++) {
                if(Integer.valueOf(catchCooldown[i].getText()) == values.staticValues.options.catch_cooldown) {
                    catchCooldown[i].setSelected(true);
                }
            }
            for(int i = 0; i < blockCooldown.length; i++) {
                if(Integer.valueOf(blockCooldown[i].getText()) == values.staticValues.options.block_cooldown) {
                    blockCooldown[i].setSelected(true);
                }
            }
        }
    };
    ActionListener notAdmin = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            for(int i = 0; i < nOfTagger.length; i++) nOfTagger[i].setEnabled(false);
            for(int i = 0; i < gameSpeed.length; i++) gameSpeed[i].setEnabled(false);
            for(int i = 0; i < sight.length; i++) sight[i].setEnabled(false);
            for(int i = 0; i < speed.length; i++) speed[i].setEnabled(false);
            for(int i = 0; i < catchCooldown.length; i++) catchCooldown[i].setEnabled(false);
            for(int i = 0; i < blockCooldown.length; i++) blockCooldown[i].setEnabled(false);
            resetBtn.setVisible(false);
            okBtn.setVisible(false);
        }
    };

    ActionListener admin = new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
            for(int i = 0; i < nOfTagger.length; i++) nOfTagger[i].setEnabled(true);
            for(int i = 0; i < gameSpeed.length; i++) gameSpeed[i].setEnabled(true);
            for(int i = 0; i < sight.length; i++) sight[i].setEnabled(true);
            for(int i = 0; i < speed.length; i++) speed[i].setEnabled(true);
            for(int i = 0; i < catchCooldown.length; i++) catchCooldown[i].setEnabled(true);
            for(int i = 0; i < blockCooldown.length; i++) blockCooldown[i].setEnabled(true);
            resetBtn.setVisible(true);
            okBtn.setVisible(true);
        }
    };
}
