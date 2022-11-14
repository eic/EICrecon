<!-- Vue 2.x  -->
<script>
  new Vue({
      el: '#example_vue',
    // Options...
      methods: {
            multiple: function (event) {
              this.hide()
              this.show()
            },
            hide:  function (event) { },
            show: function (event) { }
      },
      data() {
          return {
              // flag_name: true,
              // flag_value: false,
              flags: {},
              recoPrefixes: [],
              isHidden: false,
          };
      },
      created() {
          // Read json flags
          fetch('arches_flags.json')
              .then(response => response.json())
              .then(data => self.flags = data)
              .catch(err => console.log(err));

          // Define reconstruction flags prefixes
          this.recoPrefixes = [
              "B0ECAL",
              "B0TRK",
              "BEMC",
              "BTOF",
              "BTRK",
              "BVTX",
              "ECGEM",
              "ECTOF",
              "ECTRK",
              "EEMC",
              "FOFFMTRK",
              "HCAL",
              "MPGD",
              "RICH",
              "RPOTS",
              "ZDC",
              "Tracking",
              "Reco",
              "Digi",
              "Calorimetry"
          ];
      },
      computed: {
          filteredFlags() {
              let prefixes = this.recoPrefixes;
               return this.flags.filter(el => {
                   /*
                                                    for r in all_records
                              if r[0].casefold().startswith(reco_prefix.lower())
                              and 'LogLevel' not in r[0]
                              and 'InputTags' not in r[0]
                              and 'input_tags' not in r[0]
                              and 'verbose' not in r[0]]
                        */

                   for (let prefix of prefixes) {
                       console.log(prefix.toUpperCase());
                       if (el[0].toUpperCase().startsWith(prefix.toUpperCase())) {
                           return true;
                       }
                   }
                   return false;
               });
          },
      },
  });
</script>

<div id="example_vue">
    <button type="button" v-on:click="isHidden = !isHidden">Flag name</button>
    <button type="button" v-on:click="isHidden = !isHidden">Default value</button>
    <button type="button" v-on:click="isHidden = !isHidden">User value</button>
    <button type="button" v-on:click="isHidden = !isHidden">Description</button>
    <input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput', ['table_flags'])" placeholder="Search for names..">
    <table>
        <thead>
            <tr>
                <th v-show="!isHidden">Flag name</th>
                <th v-if="!isHidden">Default value</th>
                <th v-if="!isHidden">User value</th>
                <th v-if="!isHidden">Description</th>
            </tr>
        </thead>
        <tbody>
            <tr v-for="flag in flags">
                <td v-if="!isHidden">{{ flag[0] }}</td>
                <td v-if="!isHidden">{{ flag[1] }}</td>
                <td v-if="!isHidden">{{ flag[2] }}</td>
                <td v-if="!isHidden">{{ flag[3] }}</td>
            </tr>
        </tbody>
    </table>
</div>


<!--<script src="https://cdnjs.cloudflare.com/ajax/libs/vue/2.5.17/vue.js"></script>-->

<!--<main id="vue-app">-->
<!--  <div ref="div">Hello world</div>-->
<!--  <button type="button" v-on:click="hideElements()">Hide!</button>-->
<!--  <button type="button" v-on:click="showElements()">Show!</button>-->
<!--</main>-->

<!--<script>-->
<!--    var app = new Vue({-->
<!--        el: "#vue-app",-->
<!--        methods: {-->

<!--            hideElements: function () {-->
<!--                this.$refs.div.style.display = "none";-->
<!--            },-->
<!--            showElements: function () {-->
<!--                this.$refs.div.style.display = "inherit";-->

<!--            }-->
<!--        }-->
<!--    });-->
<!--</script>-->

